// SP_WeaponType.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "SP_WeaponType.generated.h" 

// 무기가 가질 수 있는 특수 능력 Enum
UENUM(BlueprintType)
enum class EWeaponSpecialAbility : uint8
{
    None = 0          UMETA(DisplayName = "없음"),
    Penetration = 1   UMETA(DisplayName = "총알 관통"),
    DoubleDamage = 2  UMETA(DisplayName = "대미지 증폭"),
    RapidFire = 3     UMETA(DisplayName = "연사속도 증가"),
    LongRange = 4     UMETA(DisplayName = "사거리 증가")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None = 0,
    // 기본 무기
    Pistol = 1  UMETA(DisplayName = "기본 권총"),
    Shotgun = 2  UMETA(DisplayName = "기본 샷건"),
    // 진화 무기
    Requiem = 3  UMETA(DisplayName = "진화형 권총"),
    Blast = 4  UMETA(DisplayName = "진화형 샷건"),
};

// 무기 데이터 구조체
USTRUCT(BlueprintType)
struct FWeaponData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeaponType WeaponType;

    // 현재 무기가 보유한 특수 능력 플래그
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeaponSpecialAbility ActiveAbility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EnhanceLevel = 0;

    // 기본 생성자 (인자가 없을 때 기본값 지정)
    FWeaponData()
        : WeaponType(EWeaponType::None)
        , ActiveAbility(EWeaponSpecialAbility::None)
    {
    }



    //인자를 1개만 받을 때의 생성자 (기본 무기 지급용: Standard 등)
    FWeaponData(EWeaponType InType)
        : WeaponType(InType)
        , ActiveAbility(EWeaponSpecialAbility::None)
    {
    }

    //인자를 2개 다 받을 때의 생성자
    FWeaponData(EWeaponType InType, EWeaponSpecialAbility InAbility)
        : WeaponType(InType)
        , ActiveAbility(InAbility)
    {
    }
};


// 무기 스탯 데이터 테이블 구조체
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WeaponSpread = 0.05f; // 기본 분산치 (샷건이 아니면 낮게 설정) 권총 0,01 기준 샷건0.2

    // 데이터 테이블에서 이 무기가 해금할 기본 특수 능력을 지정합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    EWeaponSpecialAbility DefaultSpecialAbility = EWeaponSpecialAbility::None;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* WeaponMesh; // 무기 외형

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Effect")
    class UNiagaraSystem* MuzzleFlash; // 총구 화염 이펙트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Effect")
    UNiagaraSystem* ImpactEffect; //타격 시 이펙트


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Effect")
    UNiagaraSystem* TracerEffect; // 탄도 궤적 이펙트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* FireSound; // 발사 소리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* ImpactSound; // 타격시 사운드

   
};