// Source/ProjectTeam16/Cube/RandomOptionComponent.cpp
#include "RandomOptionComponent.h"

URandomOptionComponent::URandomOptionComponent()
{
    CurrentGrade = EOptionGrade::Rare;
}

void URandomOptionComponent::RollOptions()
{
    // 등급 업그레이드 시도
    TryUpgradeGrade();

    // 옵션 3개 생성
    CurrentOptions.Empty();
    for (int32 i = 0; i < 3; i++)
    {
        CurrentOptions.Add(GenerateOption(i));
    }
}

void URandomOptionComponent::TryUpgradeGrade()
{
    // 등급 업그레이드 확률
    // Rare → Epic: 10%
    // Epic → Unique: 5%
    // Unique → Legendary: 1%

    int32 Roll = FMath::RandRange(1, 100);

    switch (CurrentGrade)
    {
    case EOptionGrade::Rare:
        if (Roll <= 10) CurrentGrade = EOptionGrade::Epic;
        break;
    case EOptionGrade::Epic:
        if (Roll <= 5) CurrentGrade = EOptionGrade::Unique;
        break;
    case EOptionGrade::Unique:
        if (Roll <= 3) CurrentGrade = EOptionGrade::Legendary;
        break;
    case EOptionGrade::Legendary:
        break;
    }
}

FOptionLine URandomOptionComponent::GenerateOption(int32 LineIndex)
{
    FOptionLine Option;
    Option.Line = LineIndex + 1;
    Option.OptionType = GetRandomOptionType();

    // 큰 수치 나올 확률 체크
    int32 Roll = FMath::RandRange(1, 100);
    bool bBigValue = (Roll <= GetBigValueChance(LineIndex));

    Option.Value = GetOptionValue(Option.OptionType, bBigValue);

    return Option;
}

EOptionType URandomOptionComponent::GetRandomOptionType()
{
    TMap<EOptionType, int32> WeightMap;

    // 기본 옵션
    WeightMap.Add(EOptionType::ATKUP, 80);
    WeightMap.Add(EOptionType::RangeUP, 80);
    WeightMap.Add(EOptionType::EXPRate, 70);
    WeightMap.Add(EOptionType::ATSUP, 50);
    WeightMap.Add(EOptionType::CritRate, 40);
    WeightMap.Add(EOptionType::CritDMG, 40);
    WeightMap.Add(EOptionType::PenUP, 30);
    WeightMap.Add(EOptionType::MoreCube, 5);

    // 유니크 이상 옵션
    bool bCanHaveSpecial = (CurrentGrade == EOptionGrade::Unique || CurrentGrade == EOptionGrade::Legendary);
    if (bCanHaveSpecial)
    {
        WeightMap.Add(EOptionType::BossDMG, 15);
        WeightMap.Add(EOptionType::x2chance, 10);
        WeightMap.Add(EOptionType::SplashDMG, 10);
    }
    int32 TotalWeight = 0;
    for (auto& Elem : WeightMap)
    {
        TotalWeight += Elem.Value;
    }

    int32 RandomRoll = FMath::RandRange(1, TotalWeight);
    int32 CurrentWeightSum = 0;

    for (auto& Elem : WeightMap)
    {
        CurrentWeightSum += Elem.Value;
        if (RandomRoll <= CurrentWeightSum)
        {
            return Elem.Key;
        }
    }

    return EOptionType::ATKUP; // 예외 방지용 기본값

    // 유니크 이상에서만 BossDMG, x2chance, SplashDMG 등장

    TArray<EOptionType> Pool = {
        EOptionType::ATKUP,
        EOptionType::ATSUP,
        EOptionType::RangeUP,
        EOptionType::PenUP,
        EOptionType::CritRate,
        EOptionType::CritDMG,
        EOptionType::EXPRate,
        EOptionType::MoreCube
    };

    if (bCanHaveSpecial)
    {
        Pool.Add(EOptionType::BossDMG);
        Pool.Add(EOptionType::x2chance);
        Pool.Add(EOptionType::SplashDMG);
    }

    int32 Index = FMath::RandRange(0, Pool.Num() - 1);
    return Pool[Index];
}

float URandomOptionComponent::GetOptionValue(EOptionType Type, bool bBigValue)
{
    switch (Type)
    {
    case EOptionType::MoreCube:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 100.0f : 100.0f;
        case EOptionGrade::Epic:       return bBigValue ? 200.0f : 100.0f;
        case EOptionGrade::Unique:     return bBigValue ? 300.0f : 200.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 400.0f : 300.0f;
        default: return 2.0f;
        }
    case EOptionType::ATKUP:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 3.0f : 3.0f;
        case EOptionGrade::Epic:       return bBigValue ? 6.0f : 3.0f;
        case EOptionGrade::Unique:     return bBigValue ? 9.0f : 6.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 12.0f : 9.0f;
        default: return 3.0f;
        }

    case EOptionType::ATSUP:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 7.0f : 4.0f;
        case EOptionGrade::Epic:       return bBigValue ? 10.0f : 7.0f;
        case EOptionGrade::Unique:     return bBigValue ? 15.0f : 10.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 20.0f : 15.0f;
        default: return 3.0f;
        }

    case EOptionType::RangeUP:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 7.0f : 4.0f;
        case EOptionGrade::Epic:       return bBigValue ? 10.0f : 7.0f;
        case EOptionGrade::Unique:     return bBigValue ? 15.0f : 10.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 20.0f : 15.0f;
        default: return 3.0f;
        }

    case EOptionType::PenUP:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 7.0f : 4.0f;
        case EOptionGrade::Epic:       return bBigValue ? 10.0f : 7.0f;
        case EOptionGrade::Unique:     return bBigValue ? 15.0f : 10.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 20.0f : 15.0f;
        default: return 4.0f;
        }

    case EOptionType::CritRate:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 7.0f : 4.0f;
        case EOptionGrade::Epic:       return bBigValue ? 10.0f : 7.0f;
        case EOptionGrade::Unique:     return bBigValue ? 15.0f : 10.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 20.0f : 15.0f;
        default: return 4.0f;
        }

    case EOptionType::CritDMG:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 7.0f : 4.0f;
        case EOptionGrade::Epic:       return bBigValue ? 10.0f : 7.0f;
        case EOptionGrade::Unique:     return bBigValue ? 15.0f : 10.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 20.0f : 15.0f;
        default: return 4.0f;
        }

    case EOptionType::EXPRate:
        switch (CurrentGrade)
        {
        case EOptionGrade::Rare:       return bBigValue ? 7.0f : 4.0f;
        case EOptionGrade::Epic:       return bBigValue ? 10.0f : 7.0f;
        case EOptionGrade::Unique:     return bBigValue ? 15.0f : 10.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 20.0f : 15.0f;
        default: return 4.0f;
        }
        // 4단계 수치 (유니크 이상)

    case EOptionType::SplashDMG:
        switch (CurrentGrade)
        {
        case EOptionGrade::Unique:     return bBigValue ? 10.0f : 7.0f;
        case EOptionGrade::Legendary:  return bBigValue ? 15.0f : 10.0f;
        default: return 7.0f;
        }

    case EOptionType::BossDMG:
    {
        int32 Roll = FMath::RandRange(1, 100);
        switch (CurrentGrade)
        {
        case EOptionGrade::Unique:
            if (Roll <= 5)       return 30.0f; // 4단계
            else if (Roll <= 20) return 20.0f; // 3단계
            else if (Roll <= 50) return 15.0f; // 2단계
            else                 return 10.0f; // 1단계
        case EOptionGrade::Legendary:
            if (Roll <= 5)       return 40.0f; // 4단계
            else if (Roll <= 20) return 35.0f; // 3단계
            else if (Roll <= 50) return 30.0f; // 2단계
            else                 return 20.0f; // 1단계
        default: return 10.0f;
        }
    }

    case EOptionType::x2chance:
    {
        int32 Roll = FMath::RandRange(1, 100);
        switch (CurrentGrade)
        {
        case EOptionGrade::Unique:
            if (Roll <= 5)       return 10.0f; // 4단계
            else if (Roll <= 20) return 7.0f;  // 3단계
            else if (Roll <= 50) return 5.0f;  // 2단계
            else                 return 3.0f;  // 1단계
        case EOptionGrade::Legendary:
            if (Roll <= 5)       return 15.0f; // 4단계
            else if (Roll <= 20) return 10.0f; // 3단계
            else if (Roll <= 50) return 7.0f;  // 2단계
            else                 return 5.0f;  // 1단계
        default: return 3.0f;
        }
    }

    default:
        return 3.0f;
    }
}

int32 URandomOptionComponent::GetBigValueChance(int32 LineIndex)
{
    // 1번째 슬롯: 항상 큰 값 (100%)
    // 2번째 슬롯: 낮은 확률 (25%)
    // 3번째 슬롯: 매우 낮은 확률 (10%)
    switch (LineIndex)
    {
    case 0: return 100;
    case 1: return 10;
    case 2: return 1;
    default: return 1;
    }
}