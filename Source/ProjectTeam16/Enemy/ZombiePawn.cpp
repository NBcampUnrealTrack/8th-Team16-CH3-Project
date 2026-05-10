#include "ProjectTeam16/Enemy/ZombiePawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Perception/PawnSensingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Team16PlayerController.h"

AZombiePawn::AZombiePawn()
{
    PrimaryActorTick.bCanEverTick = false;

    // 1. 캡슐 설정 (Character의 GetCapsuleComponent() 대체)
    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    SetRootComponent(CapsuleComp);
    CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
    // 좀비끼리 충돌 무시 설정
    CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 2. 메시 설정 (Character의 GetMesh() 대체)
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCastShadow(false); // 렉 방지용 그림자 제거
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 3. 이동 컴포넌트 (CharacterMovement 대체)
    MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));
    MovementComp->MaxSpeed = 300.0f;

    // 4. 기존 로직 컴포넌트
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
    AttackRangeSphere->SetupAttachment(RootComponent);
    AttackRangeSphere->SetSphereRadius(150.0f);

    AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AZombiePawn::OnAttackOverlapBegin);
    AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AZombiePawn::OnAttackOverlapEnd);
}

void AZombiePawn::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;

    if (PawnSensing)
    {
        // 0.5초마다 플레이어 감지 체크
        GetWorldTimerManager().SetTimer(SeeTimerHandle, this, &AZombiePawn::CheckVisibility, 2.0f, true);
    }
}

float AZombiePawn::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
    Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

    if (Health <= 0.0f)
    {
        bIsDead = true;
        ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(EventInstigator);
        if (PlayerController)
        {
            PlayerController->RegisterZombieKill(ExpReward);
        }
        Destroy();
    }
    return ActualDamage;
}

void AZombiePawn::CheckVisibility()
{
    APlayerController* PC = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PC) return;

    FVector CameraLoc; FRotator CameraRot;
    PC->GetPlayerViewPoint(CameraLoc, CameraRot);

    FVector ToZombie = (GetActorLocation() - CameraLoc).GetSafeNormal();
    float Dot = FVector::DotProduct(CameraRot.Vector(), ToZombie);
    float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));
    float Dist = FVector::Dist(GetActorLocation(), CameraLoc);

    if (Angle < 80.0f || Dist < 400.0f)
    {
        TargetPlayer = PC->GetPawn();
        OnSeePlayer(TargetPlayer);
        GetWorldTimerManager().ClearTimer(SeeTimerHandle);
    }
}

void AZombiePawn::OnSeePlayer(APawn* SeenPawn)
{
    if (!SeenPawn || bIsDead) return;
    TargetPlayer = SeenPawn;

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        // Pawn이어도 AIController가 붙어있고 FloatingMovement가 있으면 MoveTo 가능
        AIC->MoveToActor(TargetPlayer, 30.f);
    }
}

void AZombiePawn::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor == TargetPlayer)
    {
        AttackLoop();
        GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AZombiePawn::AttackLoop, 1.5f, true, 1.5f);
    }
}

void AZombiePawn::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == TargetPlayer)
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        if (IsValid(TargetPlayer)) OnSeePlayer(TargetPlayer);
    }
}

void AZombiePawn::AttackLoop()
{
    if (IsValid(TargetPlayer))
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastAttackTime >= 1.5f)
        {
            UGameplayStatics::ApplyDamage(TargetPlayer, DamageAmount, GetController(), this, nullptr);
            LastAttackTime = CurrentTime;
        }
    }
}