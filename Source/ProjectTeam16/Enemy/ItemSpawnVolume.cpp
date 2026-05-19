#include "ProjectTeam16/Enemy/ItemSpawnVolume.h"
#include "Components/BoxComponent.h"
#include "ItemBox.h"
#include "NavigationSystem.h"

AItemSpawnVolume::AItemSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
}

void AItemSpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle InitialSpawnHandle;
	GetWorldTimerManager().SetTimer(InitialSpawnHandle, [this]()
		{
			for (int i = 0; i < 3; ++i)
			{
				SpawnBox();
			}
		}, 0.5f, false);

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AItemSpawnVolume::SpawnBox, SpawnTime, true);
}

void AItemSpawnVolume::SpawnBox()
{
	if (!BoxClass) return;

	FVector BoxOrigin = SpawnArea->GetComponentLocation();
	FVector BoxExtent = SpawnArea->GetScaledBoxExtent();

	float RandX = FMath::FRandRange(BoxOrigin.X - BoxExtent.X, BoxOrigin.X + BoxExtent.X);
	float RandY = FMath::FRandRange(BoxOrigin.Y - BoxExtent.Y, BoxOrigin.Y + BoxExtent.Y);
	FVector RandomTargetLocation(RandX, RandY, BoxOrigin.Z);

	UNavigationSystemV1* Navi = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (Navi)
	{
		FNavLocation ProjectedLocation;
		if (Navi->GetRandomReachablePointInRadius(RandomTargetLocation, 500.0f, ProjectedLocation))
		{
			// 찾은 바닥 위치에 스폰 (바닥에 딱 붙지 않게 살짝 위로) 
			FVector FinalLocation = ProjectedLocation.Location + FVector(0.0f, 0.0f, 20.0f);
			FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

			GetWorld()->SpawnActor<AItemBox>(BoxClass, FinalLocation, SpawnRotation);
		}
	}
}




