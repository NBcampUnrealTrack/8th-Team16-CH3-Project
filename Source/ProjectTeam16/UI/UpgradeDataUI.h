#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "UpgradeDataUI.generated.h"

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
    MaxHP           UMETA(DisplayName = "최대 체력 증가"),
    HPRecover       UMETA(DisplayName = "체력 회복"),
    AttackUp        UMETA(DisplayName = "공격력 증가"),
    StaminaUp       UMETA(DisplayName = "최대 스태미나 증가"),
    WeaponEnhance   UMETA(DisplayName = "무기 강화"),
    NewWeapon       UMETA(DisplayName = "새로운 무기"),
    WeaponCombine   UMETA(DisplayName = "무기 조합"),
};

USTRUCT(BlueprintType)
struct FUpgradeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EUpgradeType UpgradeType;

    UPROPERTY(BlueprintReadWrite)
    UTexture2D* CardTexture;

    UPROPERTY(BlueprintReadWrite)
    EWeaponType WeaponTarget;

    UPROPERTY(BlueprintReadWrite)
    EWeaponType WeaponTarget2;

    UPROPERTY(BlueprintReadWrite)
    FText CardTitle;       // ← 추가: 카드 이름

    UPROPERTY(BlueprintReadWrite)
    FText CardDescription; // ← 추가: 효과 설명

    UPROPERTY(BlueprintReadWrite)
    FText CardLevel; // ← 추가: 강화 단계 

    FUpgradeData()
        : UpgradeType(EUpgradeType::MaxHP)
        , CardTexture(nullptr)
        , WeaponTarget(EWeaponType::None)
        , WeaponTarget2(EWeaponType::None)
    {
    }
};