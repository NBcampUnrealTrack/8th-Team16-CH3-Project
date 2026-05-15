// Source/ProjectTeam16/Cube/OptionTypes.h
#pragma once

#include "CoreMinimal.h"
#include "OptionTypes.generated.h"

UENUM(BlueprintType)
enum class EOptionType : uint8
{
    // 공격력, 공격속도, 사거리, 관통력, 크리확률, 크리데미지, 추가 경험치, 보스데미지, 공격 시 폭발 확률, 공격 시 2배 데미지 확률, 큐브 획득량 증가
    ATKUP,
    ATSUP,
    RangeUP,
    PenUP,
    CritRate,
    CritDMG,
    EXPRate,
    BossDMG,
    SplashDMG,
    x2chance,
    MoreCube
};

UENUM(BlueprintType)
enum class EOptionGrade : uint8
{
    Rare,
    Epic,
    Unique,
    Legendary
};

USTRUCT(BlueprintType)
struct FOptionLine
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EOptionType OptionType;

    UPROPERTY(BlueprintReadOnly)
    float Value;

    UPROPERTY(BlueprintReadOnly)
    int32 Line;


};