// Source/ProjectTeam16/Cube/RandomOptionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OptionTypes.h"
#include "RandomOptionComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTTEAM16_API URandomOptionComponent : public UObject
{
    GENERATED_BODY()

public:
    URandomOptionComponent();

    UFUNCTION(BlueprintCallable)
    void RollOptions();

    UPROPERTY(BlueprintReadOnly)
    EOptionGrade CurrentGrade;

    UPROPERTY(BlueprintReadOnly)
    TArray<FOptionLine> CurrentOptions;

private:
    // 등급 업그레이드 시도
    void TryUpgradeGrade();

    // 라인별 옵션 생성
    FOptionLine GenerateOption(int32 LineIndex);

    // 랜덤 옵션 타입 반환 (등급 고려)
    EOptionType GetRandomOptionType();

    // 옵션 수치 반환 (타입, 라인, 큰값 여부)
    float GetOptionValue(EOptionType Type, bool bBigValue);

    // 라인별 큰 수치 나올 확률 (%)
    int32 GetBigValueChance(int32 LineIndex);
};