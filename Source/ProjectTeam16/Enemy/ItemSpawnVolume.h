#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawnVolume.generated.h"

class UBoxComponent;

UCLASS()
class PROJECTTEAM16_API AItemSpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	

	AItemSpawnVolume();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<class AItemBox> BoxClass;
	UPROPERTY(VisibleAnywhere, Category="Spawn")
	TObjectPtr<UBoxComponent> SpawnArea;

	UPROPERTY(EditAnywhere, Category="Spawn")
	float SpawnTime;

	void SpawnBox();

	FTimerHandle SpawnTimerHandle;
};
