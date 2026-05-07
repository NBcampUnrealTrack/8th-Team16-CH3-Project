// SP_WeaponType.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" // 데이터 테이블 포함
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "SP_WeaponType.generated.h" 

// 1. 팀원의 무기 종류 Enum
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None = 0     UMETA(DisplayName = "없음"),
    Standard = 1 UMETA(DisplayName = "표준형 권총"),
    Old = 2      UMETA(DisplayName = "구형 권총"),
    Supply = 3   UMETA(DisplayName = "보급형 권총"),
    Spare = 4    UMETA(DisplayName = "예비용 권총"),
    Enhanced = 5 UMETA(DisplayName = "강화형 권총"),
    Improved = 6 UMETA(DisplayName = "개량형 권총"),
    Special = 7  UMETA(DisplayName = "특수형 권총")
};

// 2. 무기 데이터 구조체 (개별 무기의 상태 저장)
USTRUCT(BlueprintType)
struct FWeaponData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeaponType WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "3"))
    int32 EnhanceLevel; // 0 ~ 3

    FWeaponData() : WeaponType(EWeaponType::None), EnhanceLevel(0) {}
    FWeaponData(EWeaponType InType, int32 InLevel) : WeaponType(InType), EnhanceLevel(InLevel) {}
};

// 3. 무기 스탯 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FGunStats : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
    FString GunName = TEXT("Default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Damage = 10.0f;    //데미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FireRate = 0.2f;   //연사속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Range = 5000.0f;   //사거리


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* WeaponMesh; // 무기 외형

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Effect")
    class UNiagaraSystem* MuzzleFlash; // 총구 화염 이펙트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Effect")
    UNiagaraSystem* ImpactEffect; //타격 시 이펙트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* FireSound; // 발사 소리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* ImpactSound; // 타격시 사운드
};