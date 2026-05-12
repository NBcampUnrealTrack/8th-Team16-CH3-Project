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

