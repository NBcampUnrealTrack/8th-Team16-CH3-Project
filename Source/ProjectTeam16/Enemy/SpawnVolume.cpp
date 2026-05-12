#include "ProjectTeam16/Enemy/SpawnVolume.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "ProjectTeam16/Enemy/Zombie.h"
#include "NavigationSystem.h"

ASpawnVolume::ASpawnVolume()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SetRootComponent(SpawnArea);

}

void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	SpawnZombie();
	
	GetWorldTimerManager().SetTimer(EnrageStartTimerHandle, this, &ASpawnVolume::StartEnrage, StartEnrageTime, true);
	//설정 간격마다 좀비 스폰
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ASpawnVolume::SpawnZombie, SpawnInterval, true);

}
FVector ASpawnVolume::GetRandomPointAroundPlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return FVector::ZeroVector;

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector ForwardDir = PlayerPawn->GetActorForwardVector();

	// 기존 부채꼴 좌표 계산
	float ConeHalfAngle = 75.0f;
	float RandomAngle = FMath::RandRange(-ConeHalfAngle, ConeHalfAngle);
	FVector SpawnDir = ForwardDir.RotateAngleAxis(RandomAngle, FVector::UpVector);

	float RandomDistance = FMath::RandRange(MinRadius, MaxRadius);
	FVector RawLocation = PlayerLocation + (SpawnDir * RandomDistance);

	// 해당 좌표가 NavMesh 위인지 검증 및 보정
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		// RawLocation 주변 500유닛 이내에서 가장 가까운 NavMesh 바닥을 찾습니다.
		if (NavSys->ProjectPointToNavigation(RawLocation, ProjectedLocation, FVector(500.f, 500.f, 500.f)))
		{
			// 성공하면 보정된 좌표 반환
			return ProjectedLocation.Location;
		}
	}

	// 만약 NavMesh 바닥을 찾지 못했다면 (건물 한복판 등), 
	// 이번 스폰은 포기하거나 플레이어 위치 근처 안전한 곳을 반환합니다.
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

	//마리수 제한 
	if (CurrentZombieCount >= 100) return;

	CurrentTime = GetWorld()->GetTimeSeconds();
	int32 Elapsed30Secs = FMath::FloorToInt(CurrentTime / 30.0f);
	int32 ElapsedMinutes = FMath::FloorToInt(CurrentTime / 60.0f);

	int32 TotalWaveQuota = (Elapsed30Secs + 1) * 10;
	float ExactSpawnsPerInterval = (float)TotalWaveQuota / (30.0f / SpawnInterval) + SpawnRemainder;

	int32 SpawnsToExecute = FMath::FloorToInt(ExactSpawnsPerInterval);
	SpawnRemainder = ExactSpawnsPerInterval - SpawnsToExecute;

	if (SpawnsToExecute <= 0) return;

	// 1. 이번에 소환해야 할 엘리트 지분(10%)을 계산해서 누적합니다.
	float CurrentEliteQuota = (SpawnsToExecute * 0.1f) + EliteRemainder;

	// 2. 누적된 값에서 소환 가능한 정수 마릿수를 뽑습니다. (예: 1.2마리면 1마리)
	int32 EliteToSpawn = FMath::FloorToInt(CurrentEliteQuota);

	// 3. 소환하고 남은 소수점은 다음을 위해 저장합니다. (예: 1.2에서 1 빼고 0.2 남김)
	EliteRemainder = CurrentEliteQuota - EliteToSpawn;

	// 4. 일반 좀비는 전체 소환량에서 엘리트를 뺀 만큼 소환합니다.
	int32 NormalToSpawn = FMath::Max(0, SpawnsToExecute - EliteToSpawn);

	// 일반 좀비 스폰
	for (int32 i = 0; i < NormalToSpawn; i++)
	{
		// 마리수 제한 한 번 더 체크
		if (CurrentZombieCount >= 100) break;

		TSubclassOf<AZombie> SelectedClass = GetClassByWaveProbability(ElapsedMinutes, false);
		if (SelectedClass)
		{
			FVector Location = GetRandomPointAroundPlayer();
			if (!Location.IsZero())
			{
				AActor* Spawned = GetWorld()->SpawnActor<AZombie>(SelectedClass, Location, FRotator::ZeroRotator);
				if (Spawned)
				{
					CurrentZombieCount++; // 스폰 성공 시 카운트 증가

					AZombie* NewZombie = Cast<AZombie>(Spawned);
					if (NewZombie && bIsEnraged) 
					{
						NewZombie->SetEnrageMode(true, EnrageSpeedMultiplier);
					}
				}
			}
		}
	}

	// 엘리트 좀비 스폰
	for (int32 i = 0; i < EliteToSpawn; i++)
	{
		// 마리수 제한 한 번 더 체크
		if (CurrentZombieCount >= 100) break;

		TSubclassOf<AZombie> SelectedEliteClass = GetClassByWaveProbability(ElapsedMinutes, true);
		if (SelectedEliteClass)
		{
			FVector Location = GetRandomPointAroundPlayer();
			if (!Location.IsZero())
			{
				AActor* Spawned = GetWorld()->SpawnActor<AZombie>(SelectedEliteClass, Location, FRotator::ZeroRotator);
				if (Spawned)
				{
					CurrentZombieCount++; // 스폰 성공 시 카운트 증가
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
	bBossSpawned = true;

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
	FVector BossLocation = FVector(400.0f, -200.0f, 50.f);

	// NavMesh 위인지 확인 후 소환
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys && BossClass)
	{
		FNavLocation ProjectedLocation;
		if (NavSys->ProjectPointToNavigation(BossLocation, ProjectedLocation, FVector(1000.f, 1000.f, 1000.f)))
		{
			AZombie* Boss = GetWorld()->SpawnActor<AZombie>(BossClass, ProjectedLocation.Location, FRotator::ZeroRotator);
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
}


