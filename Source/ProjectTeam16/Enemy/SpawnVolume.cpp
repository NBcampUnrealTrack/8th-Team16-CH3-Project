#include "ProjectTeam16/Enemy/SpawnVolume.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "ProjectTeam16/Enemy/Zombie.h"

ASpawnVolume::ASpawnVolume()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SetRootComponent(SpawnArea);

}

void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	//설정 간격마다 좀비 스폰
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ASpawnVolume::SpawnZombie, SpawnInterval, true);

}
FVector ASpawnVolume::GetRandomPointAroundPlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return FVector::ZeroVector;

	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	//랜덤한 방향 뽑기
	float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	FVector RandomDirection = FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f);

	//최소 반경과 최대 반경 사이의 거리 뽑기
	float RandomDistance = FMath::RandRange(MinRadius, MaxRadius);

	//플레이어 위치 + (방향 * 거리) = 도넛모양 안의 한 점
	FVector SpawnLocation = PlayerLocation + (RandomDirection * RandomDistance);

	//플레이어와 Z높이를 동일하게 스폰
	SpawnLocation.Z = PlayerLocation.Z;

	return SpawnLocation;
}



void ASpawnVolume::SpawnZombie()
{
	if (ZombieClasses.Num() > 0) // 배열이 존재할때만 (방어코드)
	{
		// 랜덤하게 좀비 종류 하나 선택
		int32 RandomIndex = FMath::RandRange(0, ZombieClasses.Num() - 1);
		TSubclassOf<AZombie> SelectedClass = ZombieClasses[RandomIndex];

		if (SelectedClass)
		{
			FVector Location = GetRandomPointAroundPlayer();
			GetWorld()->SpawnActor<AZombie>(SelectedClass, Location, FRotator::ZeroRotator);
		}
	}
}


