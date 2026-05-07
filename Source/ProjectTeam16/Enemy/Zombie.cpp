#include "ProjectTeam16/Enemy/Zombie.h"

#include "AIController.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/PawnSensingComponent.h"
#include "Team16PlayerController.h"

AZombie::AZombie()
{
	PrimaryActorTick.bCanEverTick = false;

	MaxHealth = 100.0f;
	Health = MaxHealth;

	// 플레이어를 감지하기 위한 시야 센서입니다.
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 2000.0f;
	PawnSensing->SetPeripheralVisionAngle(45.0f);

	// 플레이어가 공격 범위에 들어오면 반복 공격 타이머를 시작합니다.
	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(RootComponent);
	AttackRangeSphere->SetSphereRadius(150.0f);

	AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AZombie::OnAttackOverlapBegin);
	AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AZombie::OnAttackOverlapEnd);
}

void AZombie::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AZombie::OnSeePlayer);
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

	if (Health <= 0.0f)
	{
		bIsDead = true;

		// 마지막 공격자가 플레이어라면 킬 카운트와 경험치 보상을 HUD에 반영합니다.
		if (ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(EventInstigator))
		{
			PlayerController->RegisterZombieKill(ExpReward);
		}

		Destroy();
	}

	return ActualDamage;
}

void AZombie::OnSeePlayer(APawn* SeenPawn)
{
	if (!SeenPawn)
	{
		return;
	}

	TargetPlayer = SeenPawn;

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->MoveToActor(SeenPawn, 5.0f);
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
		// 공격 범위 안에 있는 동안 1.5초마다 플레이어에게 데미지를 줍니다.
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AZombie::AttackLoop, 1.5f, true, 0.0f);
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
	}
}

void AZombie::AttackLoop()
{
	if (IsValid(TargetPlayer))
	{
		UGameplayStatics::ApplyDamage(TargetPlayer, DamageAmount, GetController(), this, nullptr);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Zombie Attack!"));
	}
}
