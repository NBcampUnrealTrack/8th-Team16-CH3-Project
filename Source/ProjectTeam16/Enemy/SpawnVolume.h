#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

class UBoxComponent;

UCLASS()
class PROJECTTEAM16_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ASpawnVolume();

	//스폰 랜덤 위치 계산 함수
	FVector GetRandomPointInVolume();

protected:
	
	virtual void BeginPlay() override;

	//좀비 스폰 영역
	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	TObjectPtr<UBoxComponent> SpawnArea;
	//좀비 종류 (블루프린트 선택)
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<TSubclassOf<class AZombie>> ZombieClasses;
	//좀비 스폰 텀
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float SpawnInterval = 2.0f;

	//좀비 스폰 구현 함수
	void SpawnZombie();

	FTimerHandle SpawnTimerHandle;


};
