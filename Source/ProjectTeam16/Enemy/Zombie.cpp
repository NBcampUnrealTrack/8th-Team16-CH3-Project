#include "ProjectTeam16/Enemy/Zombie.h"
#include "AIController.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Team16PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "SpawnVolume.h"
#include "Engine/DamageEvents.h"
#include "NavigationSystem.h"
// 💡 빌드 에러 해결을 위해 추가된 필수 헤더 파일들
#include "ProjectTeam16/Data/ProjectDataStructs.h"       // FZombieStatData 위치
#include "Navigation/PathFollowingComponent.h"           // FPathFollowingRequestResult 위치
#include "AITypes.h"                                     // FAIMoveRequest 위치

AZombie::AZombie()
{
    PrimaryActorTick.bCanEverTick = false;

    AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
    AttackRangeSphere->SetupAttachment(RootComponent);

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        Capsule->SetGenerateOverlapEvents(false);
        Capsule->bDynamicObstacle = false;
    }

    if (GetMesh())
    {
        GetMesh()->SetGenerateOverlapEvents(false);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetMesh()->CastShadow = false;
    }

    AttackRangeSphere->SetSphereRadius(150.0f);
    AttackRangeSphere->SetGenerateOverlapEvents(true);

    AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AZombie::OnAttackOverlapBegin);
    AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AZombie::OnAttackOverlapEnd);
}

void AZombie::BeginPlay()
{
    Super::BeginPlay();

    if (!ZombieStatTable)
    {
        ZombieStatTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_ZombieStat.DT_ZombieStat"));
    }

    if (StatRowName.IsNone())
    {
        FString InferredRowName = GetClass() ? GetClass()->GetName() : GetName();
        InferredRowName.RemoveFromEnd(TEXT("_C"));
        InferredRowName.RemoveFromStart(TEXT("BP_"));
        if (InferredRowName.StartsWith(TEXT("Zombie")))
        {
            StatRowName = FName(*InferredRowName);
        }
    }

    if (ZombieStatTable && !StatRowName.IsNone())
    {
        FZombieStatData* StatData = ZombieStatTable->FindRow<FZombieStatData>(StatRowName, TEXT(""));
        if (StatData)
        {
            MaxHealth = StatData->MaxHealth;
            Health = MaxHealth;
            DamageAmount = StatData->Damage;
            ExpAmount = StatData->ExpReward;

            if (GetCharacterMovement())
            {
                GetCharacterMovement()->MaxWalkSpeed = StatData->MoveSpeed;
            }
        }
    }
    GetWorldTimerManager().SetTimer(SeeTimerHandle, this, &AZombie::CheckVisibility, 0.2f, true);
}

float AZombie::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
    FVector ActualHitLocation = GetActorLocation();

    if (HitSound) UGameplayStatics::PlaySoundAtLocation(this, HitSound, ActualHitLocation);
    if (HitParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, ActualHitLocation, FRotator::ZeroRotator);

    Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

    if (ActorHasTag(TEXT("Boss")))
    {
        if (ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PlayerController->UpdateHUDBossHealth(Health, MaxHealth);
        }
    }

    if (Health <= 0.0f)
    {
        bIsDead = true;

        // AI 및 타이머 즉시 정지 (몽타주 방해 금지)
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        GetWorldTimerManager().ClearTimer(SeeTimerHandle);
        GetWorldTimerManager().ClearTimer(ResetChaseTimerHandle);

        if (AAIController* AIC = Cast<AAIController>(GetController())) AIC->StopMovement();
        if (GetCharacterMovement()) GetCharacterMovement()->DisableMovement();

        const bool bWasBoss = ActorHasTag(TEXT("Boss"));

        AActor* FoundSpawner = UGameplayStatics::GetActorOfClass(GetWorld(), ASpawnVolume::StaticClass());
        if (ASpawnVolume* Spawner = Cast<ASpawnVolume>(FoundSpawner))
        {
            Spawner->OnZombieDestroyed();
        }

        ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(EventInstigator);
        if (!PlayerController && DamageCauser)
        {
            if (APawn* OwnerPawn = Cast<APawn>(DamageCauser->GetOwner()))
            {
                PlayerController = Cast<ATeam16PlayerController>(OwnerPawn->GetController());
            }
        }

        if (PlayerController) PlayerController->RegisterZombieKill(ExpAmount);

        if (bWasBoss)
        {
            if (!PlayerController) PlayerController = Cast<ATeam16PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
            if (PlayerController) PlayerController->StartBossClearSequence();
        }

        // 즉시 Destroy하지 않고, 애니메이션이 재생될 최소 1.0초의 유예를 준 뒤 삭제
        GetWorldTimerManager().SetTimer(ResetChaseTimerHandle, this, &AZombie::HandleDeathCleanup, 1.0f, false);
    }

    return ActualDamage;
}

void AZombie::HandleDeathCleanup()
{
    Destroy();
}

void AZombie::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsDead) return;
    if (OtherActor && OtherActor == TargetPlayer)
    {
        if (!GetWorldTimerManager().IsTimerActive(AttackTimerHandle))
        {
            AttackLoop();
            GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AZombie::AttackLoop, 1.5f, true, 1.5f);
        }
    }
}

void AZombie::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (bIsDead) return;
    if (OtherActor && OtherActor == TargetPlayer)
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        if (IsValid(TargetPlayer)) ChasePlayer();
    }
}

void AZombie::AttackLoop()
{
    if (bIsDead || !IsValid(TargetPlayer)) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime >= 1.5f)
    {
        UGameplayStatics::ApplyDamage(TargetPlayer, DamageAmount, GetController(), this, nullptr);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Zombie Attack!"));
        LastAttackTime = CurrentTime;
    }
}

void AZombie::CheckVisibility()
{
    if (bIsDead) return;
    APlayerController* PC = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PC) return;

    FVector CameraLocation; FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector CameraForward = CameraRotation.Vector();
    FVector ToZombie = (GetActorLocation() - CameraLocation).GetSafeNormal();

    float DotProduct = FVector::DotProduct(CameraForward, ToZombie);
    float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
    float Distance = FVector::Dist(GetActorLocation(), CameraLocation);

    if (Angle < 80.0f || Distance < 400.0f)
    {
        TargetPlayer = PC->GetPawn();
        if (TargetPlayer && !GetWorldTimerManager().IsTimerActive(ResetChaseTimerHandle))
        {
            ChasePlayer();
        }
    }
}

void AZombie::SetEnrageMode(bool bIsEnraged, float SpeedMultiplier)
{
    if (bIsDead || !GetCharacterMovement()) return;
    if (bIsEnraged) GetCharacterMovement()->MaxWalkSpeed *= SpeedMultiplier;
    else GetCharacterMovement()->MaxWalkSpeed /= SpeedMultiplier;
}

void AZombie::ChasePlayer()
{
    if (!TargetPlayer || bIsDead) return;
    AAIController* AIController = Cast<AAIController>(GetController());
    if (!AIController) return;

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalActor(TargetPlayer);
    MoveRequest.SetAcceptanceRadius(60.0f);
    MoveRequest.SetAllowPartialPath(true);

    // 💡 언리얼 엔진 5 공식 규격에 맞춘 깔끔하고 정확한 결과 판정
    FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest);

    // 이미 목표(플레이어) 위치에 도달해 있는 상태인 경우
    if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        RetryChaseWithDelay();
    }
    // 장애물이나 네비메시 단절 등으로 인해 추적 명령 자체가 실패한 경우
    else if (Result.Code == EPathFollowingRequestResult::Failed)
    {
        MoveToNearbyTarget();
    }
}



void AZombie::MoveToNearbyTarget()
{
    if (!TargetPlayer || bIsDead) return;
    AAIController* AIController = Cast<AAIController>(GetController());
    if (!AIController) return;

    FVector PlayerLocation = TargetPlayer->GetActorLocation();
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        FNavLocation PredictedLocation;
        if (NavSys->GetRandomReachablePointInRadius(PlayerLocation, 350.0f, PredictedLocation))
        {
            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(PredictedLocation.Location);
            MoveRequest.SetAcceptanceRadius(60.0f);
            MoveRequest.SetAllowPartialPath(true);
            AIController->MoveTo(MoveRequest);
        }
    }
    RetryChaseWithDelay();
}

void AZombie::RetryChaseWithDelay()
{
    if (bIsDead) return;
    GetWorldTimerManager().SetTimer(ResetChaseTimerHandle, this, &AZombie::ChasePlayer, 0.1f, false);
}
