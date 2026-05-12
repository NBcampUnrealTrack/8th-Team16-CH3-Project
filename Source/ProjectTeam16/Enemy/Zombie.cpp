#include "ProjectTeam16/Enemy/Zombie.h"

#include "AIController.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/PawnSensingComponent.h"
#include "Team16PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "Components\CapsuleComponent.h"
#include "SpawnVolume.h"
#include "ProjectTeam16/UI/BossHealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "ProjectTeam16/Data/ProjectDataStructs.h"

AZombie::AZombie()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(RootComponent);

	//HealthBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	//HealthBarWidgetComp->SetupAttachment(RootComponent);
	//HealthBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); // 머리 위 위치
	//HealthBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // 화면에 2D로 표시

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

	if (PawnSensing)
	{
		PawnSensing->SensingInterval = 2.0f;
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

	if (ZombieStatTable) 
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

	//if (ActorHasTag(FName("Boss")))
	//{
	//	SetupBossUI();
	//}
	//else
	//{
	//	HealthBarWidgetComp->SetVisibility(false); // 일반 좀비는 숨김
	//}

	if (PawnSensing)
	{
		GetWorldTimerManager().SetTimer(SeeTimerHandle, this, &AZombie::CheckVisibility, 1.0f, true);
	}
}

float AZombie::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	Health -= ActualDamage;
	Health = FMath::Clamp(Health, 0.0f, MaxHealth);

	//if (BossHealthBar)
	//{
	//	BossHealthBar->UpdateHealthBar(Health, MaxHealth);
	//}

	if (Health <= 0.0f)
	{
		bIsDead = true;

		// 월드에서 SpawnVolume을 찾아 카운트를 줄여줍니다.
		AActor* FoundSpawner = UGameplayStatics::GetActorOfClass(GetWorld(), ASpawnVolume::StaticClass());
		ASpawnVolume* Spawner = Cast<ASpawnVolume>(FoundSpawner);
		if (Spawner)
		{
			Spawner->CurrentZombieCount--;
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

void AZombie::OnSeePlayer(APawn* SeenPawn)
{
	if (!SeenPawn || bIsDead) return;

	TargetPlayer = SeenPawn;

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->MoveToActor(SeenPawn, 50.0f);
	}
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
			OnSeePlayer(TargetPlayer);
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
	// 1. 플레이어의 '컨트롤러'를 가져옵니다 (카메라 제어권자)
	APlayerController* PC = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!PC) return;

	// 2. 카메라의 실제 위치와 바라보는 방향(화살표)을 가져옵니다.
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector CameraForward = CameraRotation.Vector(); // 카메라가 뚫어지게 보는 방향

	// 3. 카메라에서 나(좀비)를 향하는 화살표를 만듭니다.
	FVector ToZombie = (GetActorLocation() - CameraLocation).GetSafeNormal();

	// 4. 두 화살표 사이의 일치도(내적)를 구합니다.
	float DotProduct = FVector::DotProduct(CameraForward, ToZombie);

	// 각도로 변환 (0도에 가까울수록 시선 정중앙)
	float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	// 5. 거리 측정
	float Distance = FVector::Dist(GetActorLocation(), CameraLocation);

	// 6. 판단 (시야각 80도 이내거나 거리가 400 이내면 발동!)
	if (Angle < 80.0f || Distance < 400.0f)
	{
		TargetPlayer = PC->GetPawn();
		OnSeePlayer(TargetPlayer);

		GetWorldTimerManager().ClearTimer(SeeTimerHandle);
	}
}

//void AZombie::SetupBossUI()
//{
//	if (HealthBarWidgetComp)
//	{
//		// 위젯 컴포넌트에서 실제 위젯 객체를 가져와 캐스팅
//		BossHealthBar = Cast<UBossHealthBarWidget>(HealthBarWidgetComp->GetWidget());
//
//		if (BossHealthBar)
//		{
//			BossHealthBar->SetBossName(TEXT("Titan Zombie")); // 이름 설정
//			BossHealthBar->UpdateHealthBar(Health, MaxHealth); // 초기 체력 설정
//		}
//	}
//}

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




