#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProjectDataStructs.generated.h"

USTRUCT(BlueprintType)
struct FZombieStatData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float MoveSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	int32 ExpReward;
};

USTRUCT(BlueprintType)
struct FPlayerGrowthData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	float NextLevelExp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	float MoveSpeed;
};

class AItem;

USTRUCT(BlueprintType)
struct FBoxDropData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AItem> ItemClass; // 스폰할 아이템 클래스 (화약, 포션 등)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DropChance; // 드롭 확률 (예: 0.1 이면 10%)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName; // 아이템 이름 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealingAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExpAmount;
};