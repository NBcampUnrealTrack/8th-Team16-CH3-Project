#include "ProjectTeam16/Enemy/Zombie.h"

#include "AIController.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Team16PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "Components\CapsuleComponent.h"
#include "SpawnVolume.h"
#include "ProjectTeam16/UI/BossHealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "ProjectTeam16/Data/ProjectDataStructs.h"
#include "Engine/DamageEvents.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "AITypes.h"

AZombie::AZombie()
{
	PrimaryActorTick.bCanEverTick = false;

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(RootComponent);

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		Capsule->SetGenerateOverlapEvents(false); // 캡슐 오버랩 끔
		Capsule->bDynamicObstacle = false;
	}

	if (GetMesh())
	{
		GetMesh()->SetGenerateOverlapEvents(false); // 메시 오버랩 끔
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->CastShadow = false;
	}

	// 공격 범위 설정 (이 구체는 플레이어를 감지해야 하므로 오버랩 유지)
	AttackRangeSphere->SetSphereRadius(150.0f);
	AttackRangeSphere->SetGenerateOverlapEvents(true);
	// 좀비끼리 무시해도 이 구체는 플레이어(Pawn)를 감지하도록 설정되어야 함

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
	if (bIsDead)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	FVector ActualHitLocation = GetActorLocation(); 

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ActualHitLocation);
	}

	// 파티클 
	if (HitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, ActualHitLocation, FRotator::ZeroRotator);
	}

	Health -= ActualDamage;
	Health = FMath::Clamp(Health, 0.0f, MaxHealth);

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

		// 월드에서 SpawnVolume을 찾아 카운트를 줄여줍니다.
		AActor* FoundSpawner = UGameplayStatics::GetActorOfClass(GetWorld(), ASpawnVolume::StaticClass());
		ASpawnVolume* Spawner = Cast<ASpawnVolume>(FoundSpawner);
		if (Spawner)
		{
			Spawner->OnZombieDestroyed();
		}

		// 마지막 공격자가 플레이어라면 킬 카운트와 경험치 보상을 HUD에 반영합니다.
		ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(EventInstigator);
		if (!PlayerController && DamageCauser)
		{
			if (APawn* OwnerPawn = Cast<APawn>(DamageCauser->GetOwner()))
			{
				PlayerController = Cast<ATeam16PlayerController>(OwnerPawn->GetController());
			}
		}

		// 공격자 컨트롤러를 찾았을 때만 킬 카운트와 경험치를 지급합니다.
		if (PlayerController)
		{
			PlayerController->RegisterZombieKill(ExpAmount);
		}
		
		Destroy();
	}

	return ActualDamage;
}

void AZombie::OnAttackOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (OtherActor && OtherActor == TargetPlayer)
	{
		// 이미 공격 타이머가 돌고 있다면 새로 만들지 않음
		if (!GetWorldTimerManager().IsTimerActive(AttackTimerHandle))
		{
			AttackLoop();
			GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AZombie::AttackLoop, 1.5f, true, 1.5f);
		}
	}
}

void AZombie::OnAttackOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (OtherActor && OtherActor == TargetPlayer)
	{
		GetWorldTimerManager().ClearTimer(AttackTimerHandle);

		if (IsValid(TargetPlayer))
		{
			ChasePlayer();
		}
	}
}

void AZombie::AttackLoop()
{
	if (IsValid(TargetPlayer))
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();

		// 마지막 공격으로부터 1.5초가 지났을 때만 실제 데미지 적용
		if (CurrentTime - LastAttackTime >= 1.5f)
		{
			UGameplayStatics::ApplyDamage(TargetPlayer, DamageAmount, GetController(), this, nullptr);
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Zombie Attack!"));

			LastAttackTime = CurrentTime; // 마지막 공격 시간 갱신
		}
	}
}

void AZombie::CheckVisibility()
{
	APlayerController* PC = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!PC) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector CameraForward = CameraRotation.Vector();
	FVector ToZombie = (GetActorLocation() - CameraLocation).GetSafeNormal();

	float DotProduct = FVector::DotProduct(CameraForward, ToZombie);
	float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
	float Distance = FVector::Dist(GetActorLocation(), CameraLocation);

	// 시야각 80도 이내거나 거리가 400 이내일 때 플레이어 포착
	if (Angle < 80.0f || Distance < 400.0f)
	{
		TargetPlayer = PC->GetPawn();

		// 중복 호출하지 않도록 방어하고, 처음 발견했을 때만 추격
		if (TargetPlayer && !GetWorldTimerManager().IsTimerActive(ResetChaseTimerHandle))
		{
			ChasePlayer();
		}
	}
}


void AZombie::SetEnrageMode(bool bIsEnraged, float SpeedMultiplier)
{
	if (GetCharacterMovement())
	{
		if (bIsEnraged)
		{
			GetCharacterMovement()->MaxWalkSpeed *= SpeedMultiplier;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed /= SpeedMultiplier;
		}
	}

	if (bIsEnraged)
	{
		// 시각적 피드백:메쉬 색을 붉게 바꾸거나 이펙트 부착
	}
	else
	{
		// 원래 색으로 복구
	}
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

	FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest);

	// 추적 성공 
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		RetryChaseWithDelay();
	}
	// 추적 실패
	else if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		MoveToNearbyTarget();
	}
}

void AZombie::MoveToNearbyTarget()
{
	if (!TargetPlayer || bIsDead) return;

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController ) return;

	// 플레이어 좌표 확보
	FVector PlayerLocation = TargetPlayer->GetActorLocation();
	FVector RandomReachableLocation;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation PredictedLocation;
		// 반지름 350.0f 내에서 이동 가능한 무작위 위치 찾기
		bool bFoundPoint = NavSys->GetRandomReachablePointInRadius(PlayerLocation, 350.0f, PredictedLocation);

		if (bFoundPoint)
		{
			RandomReachableLocation = PredictedLocation.Location; // 실제 좌표 추출

			FAIMoveRequest MoveRequest;
			MoveRequest.SetGoalLocation(RandomReachableLocation);
			MoveRequest.SetAcceptanceRadius(60.0f);
			MoveRequest.SetAllowPartialPath(true);

			AIController->MoveTo(MoveRequest);
		}
	}

	// 성공 여부와 상관없이 무한 루프 방지용 딜레이 후 복귀
	RetryChaseWithDelay();
}

void AZombie::RetryChaseWithDelay()
{
	if (bIsDead) return;

	// 딜레이로 스택 오버플로우나 렉 방지
	GetWorldTimerManager().SetTimer(
		ResetChaseTimerHandle,
		this,
		&AZombie::ChasePlayer,
		0.1f,
		false
	);
}



