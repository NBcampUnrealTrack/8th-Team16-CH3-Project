#include "ProjectTeam16/Enemy/SpawnVolume.h"
#include "Kismet/KismetMathLibrary.h"
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
FVector ASpawnVolume::GetRandomPointInVolume()
{
	FVector Origin = SpawnArea->GetComponentLocation();
	FVector Extent = SpawnArea->GetScaledBoxExtent();

	//박스 범위안에서 랜덤한 좌표값 반환
	return UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
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
			FVector Location = GetRandomPointInVolume();
			GetWorld()->SpawnActor<AZombie>(SelectedClass, Location, FRotator::ZeroRotator);
		}
	}
}


