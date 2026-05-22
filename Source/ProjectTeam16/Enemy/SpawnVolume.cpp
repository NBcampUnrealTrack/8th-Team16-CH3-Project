#include "ProjectTeam16/Enemy/SpawnVolume.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "ProjectTeam16/Enemy/Zombie.h"
#include "NavigationSystem.h"
#include "Team16PlayerController.h"

ASpawnVolume::ASpawnVolume()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SetRootComponent(SpawnArea);

}

void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(EnrageStartTimerHandle, this, &ASpawnVolume::StartEnrage, StartEnrageTime, true);
	//설정 간격마다 좀비 스폰
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ASpawnVolume::SpawnZombie, SpawnInterval, true);

}
FVector ASpawnVolume::GetRandomPointAroundPlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return FVector::ZeroVector;

	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	// 0도 ~ 360도 사이의 랜덤한 각도추출
	float RandomAngleDegree = FMath::RandRange(0.0f, 360.0f);
	float RandomAngleRadian = FMath::DegreesToRadians(RandomAngleDegree);

	// 도넛의 최소 반경과 최대 반경 사이의 무작위 거리 추출
	float RandomDistance = FMath::RandRange(MinRadius, MaxRadius);
	float SpawnX = PlayerLocation.X + (FMath::Cos(RandomAngleRadian) * RandomDistance);
	float SpawnY = PlayerLocation.Y + (FMath::Sin(RandomAngleRadian) * RandomDistance);

	// Z축은 플레이어와 동일한 높이로 세팅
	FVector RawLocation = FVector(SpawnX, SpawnY, PlayerLocation.Z);

	// 해당 좌표가 NavMesh 위인지 검증 
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		// RawLocation 주변 500유닛 이내에서 가장 가까운 NavMesh 바닥을 찾음
		if (NavSys->ProjectPointToNavigation(RawLocation, ProjectedLocation, FVector(500.f, 500.f, 500.f)))
		{
			// 검증 완료된 바닥 좌표 반환
			return ProjectedLocation.Location;
		}
	}

	// 만약 NavMesh 바닥이 없는 허공이나 벽 내부라면 제로 벡터 반환 (스폰 X)
	return FVector::ZeroVector;
}



TSubclassOf<AZombie> ASpawnVolume::GetClassByWaveProbability(int32 CurrentMinutes, bool bIsElite)
{
	const TArray<TSubclassOf<AZombie>>& TargetArray = bIsElite ? EliteClasses : ZombieClasses;
	if (TargetArray.Num() == 0) return nullptr;

	// 현재 등장 가능한 최대 인덱스
	int32 MaxWaveIdx = FMath::Min(CurrentMinutes, TargetArray.Num() - 1);

	float RandVal = FMath::FRand();
	float CumulativeProbability = 0.0f;
	float CurrentChance = 0.5f; // 최신 몹은 50% 확률

	// 최신 몹부터 역순으로 확률 체크
	for (int32 i = MaxWaveIdx; i >= 0; i--)
	{
		// 0번(가장 오래된) 몹까지 오면 남은 확률 몰아주기
		if (i == 0) return TargetArray[0];

		CumulativeProbability += CurrentChance;
		if (RandVal <= CumulativeProbability)
		{
			return TargetArray[i];
		}

		CurrentChance *= 0.5f; // 이전 단계로 갈수록 확률 절반 감소
	}

	return TargetArray[MaxWaveIdx];
}

void ASpawnVolume::SpawnZombie()
{
	CurrentTime = GetWorld()->GetTimeSeconds();

	// 10분(600초)이 되면 일반 스폰 로직을 아예 실행하지 않음
	if (CurrentTime >= BossSpawnTime)
	{
		if (!bBossSpawned)
		{
			SpawnBoss();
		}
		return;
	}

	// 배열 방어 코드
	if (ZombieClasses.Num() == 0 && EliteClasses.Num() == 0) return;

	if (CurrentZombieCount < MaxZombieCount)
	{
		int32 ElapsedMinutes = FMath::FloorToInt(CurrentTime / 60.0f);

		TotalSpawnCount++;

		// 엘리트 확률 계산 및 클래스 선택 
		bool bSpawnElite = (TotalSpawnCount % 10 == 0);
		TSubclassOf<AZombie> SelectedClass = GetClassByWaveProbability(ElapsedMinutes, bSpawnElite);

		if (SelectedClass)
		{
			FVector Location = GetRandomPointAroundPlayer();
			if (!Location.IsZero())
			{
				AActor* Spawned = GetWorld()->SpawnActor<AZombie>(SelectedClass, Location, FRotator::ZeroRotator);
				if (Spawned)
				{
					CurrentZombieCount++;

					AZombie* NewZombie = Cast<AZombie>(Spawned);
					if (NewZombie && bIsEnraged)
					{
						NewZombie->SetEnrageMode(true, EnrageSpeedMultiplier);
					}
				}
			}
		}
	}
}


void ASpawnVolume::SpawnBoss()
{
	// 기존에 있는 모든 좀비 제거 
	TArray<AActor*> FoundZombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombie::StaticClass(), FoundZombies);
	for (AActor* Zombie : FoundZombies)
	{
		Zombie->Destroy();
	}
	// 카운트 초기화
	CurrentZombieCount = 0;

	// 맵 중앙 좌표 설정 
	FVector BossLocation = FVector(-3750.0f, 9070.0f, 150.f);

	// NavMesh 위인지 확인 후 소환
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys && BossClass)
	{
		FNavLocation ProjectedLocation;
		if (NavSys->ProjectPointToNavigation(BossLocation, ProjectedLocation, FVector(1000.f, 1000.f, 1000.f)))
		{
			AZombie* Boss = GetWorld()->SpawnActor<AZombie>(BossClass, ProjectedLocation.Location, FRotator::ZeroRotator);
			if (Boss)
			{
				bBossSpawned = true;
				Boss->Tags.AddUnique(TEXT("Boss"));

				if (ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
				{
					PlayerController->ShowHUDBossHealth(TEXT("Titan Zombie"), Boss->GetCurrentHealth(), Boss->GetMaxHealth());
				}
			}
		}
	}
}

void ASpawnVolume::StartEnrage()
{
	bIsEnraged = true;

	// 현재 맵에 스폰되어 있는 모든 좀비를 찾아서 강화
	TArray<AActor*> FoundZombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombie::StaticClass(), FoundZombies);

	for (AActor* Actor : FoundZombies)
	{
		AZombie* Zombie = Cast<AZombie>(Actor);
		if (Zombie)
		{
			// 좀비 클래스에 만들 강화 함수 호출
			Zombie->SetEnrageMode(true, EnrageSpeedMultiplier);
		}
	}

	ReceiveOnStartEnrage();

	// 15초 뒤에 종료 함수를 호출하는 일회성 타이머 설정
	GetWorldTimerManager().SetTimer(EnrageDurationTimerHandle, this, &ASpawnVolume::EndEnrage, EnrageTime, false);
}

void ASpawnVolume::EndEnrage()
{
	bIsEnraged = false;
	
	// 모든 좀비를 다시 정상 상태로
	TArray<AActor*> FoundZombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombie::StaticClass(), FoundZombies);

	for (AActor* Actor : FoundZombies)
	{
		AZombie* Zombie = Cast<AZombie>(Actor);
		if (Zombie)
		{
			Zombie->SetEnrageMode(false, EnrageSpeedMultiplier);
		}
	}

	ReceiveOnEndEnrage();
}

void ASpawnVolume::OnZombieDestroyed()
{
	CurrentZombieCount = FMath::Max(0, CurrentZombieCount - 1);
	SpawnZombie();
}



