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
	FVector GetRandomPointAroundPlayer();

	int32 CurrentZombieCount = 0;
protected:
	
	virtual void BeginPlay() override;

	//좀비 스폰 영역
	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	TObjectPtr<UBoxComponent> SpawnArea;

	//좀비 종류 (블루프린트 선택)
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<TSubclassOf<class AZombie>> ZombieClasses; //일반좀비
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<TSubclassOf<class AZombie>> EliteClasses; //엘리트좀비
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AZombie> BossClass;					 //보스좀비

	//좀비 스폰 텀, 스폰 범위조절
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float SpawnInterval = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float MinRadius = 800.0f; //도넛 모양의 스폰범위에서 안쪽 원
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float MaxRadius = 1500.0f; //도넛 모양의 스폰범위에서 바깥쪽 원
	//소수점 좀비 수 확인용
	float SpawnRemainder = 0.0f;
	float EliteRemainder = 0.0f;

	// 확률 기반 클래스 선택 함수
	TSubclassOf<AZombie> GetClassByWaveProbability(int32 CurrentMinutes, bool bIsElite);

	//좀비 스폰 구현 함수
	void SpawnZombie();

	bool bBossSpawned = false; // 보스 소환 여부
	void SpawnBoss();          // 보스 소환 함수

	FTimerHandle SpawnTimerHandle;

	float CurrentTime;
};
