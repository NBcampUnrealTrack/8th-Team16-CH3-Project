#include "ProjectTeam16/UI/LevelUpWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"

void ULevelUpWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CardButton1) CardButton1->OnClicked.AddDynamic(this, &ULevelUpWidget::OnCard1Clicked);
    if (CardButton2) CardButton2->OnClicked.AddDynamic(this, &ULevelUpWidget::OnCard2Clicked);
    if (CardButton3) CardButton3->OnClicked.AddDynamic(this, &ULevelUpWidget::OnCard3Clicked);
    if (CardButton4) CardButton4->OnClicked.AddDynamic(this, &ULevelUpWidget::OnCard4Clicked);
}

UTexture2D* ULevelUpWidget::LoadCardTexture(const FString& TexturePath)
{
    UTexture2D* Texture = Cast<UTexture2D>(
        StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePath));

    if (!Texture)
    {
        UE_LOG(LogTemp, Error, TEXT("Texture Load Failed: %s"), *TexturePath);
    }
    return Texture;
}

TArray<FUpgradeData> ULevelUpWidget::BuildUpgradePool(ASP_Character* PlayerChar)
{
    TArray<FUpgradeData> Pool;
    if (!PlayerChar) return Pool;

    // 스탯 카드 (항상 등장)
    auto MakeStatCard = [&](EUpgradeType Type, const FString& Path)
        {
            FUpgradeData Data;
            Data.UpgradeType = Type;
            Data.CardTexture = LoadCardTexture(Path);
            Pool.Add(Data);
        };

    MakeStatCard(EUpgradeType::MaxHP,
        TEXT("/Game/UI/InGameUI/png/CurHPUP-Photoroom"));
    MakeStatCard(EUpgradeType::HPRecover,
        TEXT("/Game/UI/InGameUI/png/HPgenerate-Photoroom"));
    MakeStatCard(EUpgradeType::AttackUp,
        TEXT("/Game/UI/InGameUI/png/ATKUP-Photoroom"));

    // 강화 경로 맵
    TMap<EWeaponType, FString> EnhancePathMap;
    EnhancePathMap.Add(EWeaponType::Standard, TEXT("/Game/UI/InGameUI/png/1gunpuls-Photoroom"));
    EnhancePathMap.Add(EWeaponType::Old, TEXT("/Game/UI/InGameUI/png/2gunplus-Photoroom"));
    EnhancePathMap.Add(EWeaponType::Supply, TEXT("/Game/UI/InGameUI/png/3gunplus-Photoroom"));
    EnhancePathMap.Add(EWeaponType::Spare, TEXT("/Game/UI/InGameUI/png/4gunplus-Photoroom"));
    EnhancePathMap.Add(EWeaponType::Enhanced, TEXT("/Game/UI/InGameUI/png/Alphagun-Photoroom"));
    EnhancePathMap.Add(EWeaponType::Improved, TEXT("/Game/UI/InGameUI/png/BetaGun-Photoroom"));
    EnhancePathMap.Add(EWeaponType::Special, TEXT("/Game/UI/InGameUI/png/Specialgun-Photoroom"));

    // 새 무기 경로 맵
    TMap<EWeaponType, FString> NewWeaponPathMap;
    NewWeaponPathMap.Add(EWeaponType::Old, TEXT("/Game/UI/InGameUI/png/2gun-Photoroom"));
    NewWeaponPathMap.Add(EWeaponType::Supply, TEXT("/Game/UI/InGameUI/png/3gun-Photoroom"));
    NewWeaponPathMap.Add(EWeaponType::Spare, TEXT("/Game/UI/InGameUI/png/4gun"));

    // 조합 해금 여부
    bool bHasImproved = PlayerChar->HasWeapon(EWeaponType::Improved);
    bool bHasEnhanced = PlayerChar->HasWeapon(EWeaponType::Enhanced);
    bool bHasSpecial = PlayerChar->HasWeapon(EWeaponType::Special);

    // 강화 카드 (소지 중 + 3강 미만)
    TArray<EWeaponType> EnhanceOrder = {
        EWeaponType::Standard, EWeaponType::Old,
        EWeaponType::Supply,   EWeaponType::Spare,
        EWeaponType::Enhanced, EWeaponType::Improved,
        EWeaponType::Special
    };

    for (EWeaponType Type : EnhanceOrder)
    {
        if (!PlayerChar->HasWeapon(Type)) continue;
        int32 Level = PlayerChar->GetWeaponEnhanceLevel(Type);
        if (Level < 0 || Level >= 3) continue;

        FUpgradeData Enhance;
        Enhance.UpgradeType = EUpgradeType::WeaponEnhance;
        Enhance.WeaponTarget = Type;
        Enhance.CardTexture = LoadCardTexture(EnhancePathMap[Type]);
        Pool.Add(Enhance);
        UE_LOG(LogTemp, Warning, TEXT("Enhance Added: Type%d Level%d"), (int32)Type, Level);
    }

    // 새 무기 카드
    // 개량형 OR 특수형 해금 시 표준형+구형 계열 제거
    if (!bHasImproved && !bHasSpecial)
    {
        if (!PlayerChar->HasWeapon(EWeaponType::Old))
        {
            FUpgradeData NewOld;
            NewOld.UpgradeType = EUpgradeType::NewWeapon;
            NewOld.WeaponTarget = EWeaponType::Old;
            NewOld.CardTexture = LoadCardTexture(NewWeaponPathMap[EWeaponType::Old]);
            Pool.Add(NewOld);
            UE_LOG(LogTemp, Warning, TEXT("NewWeapon Added: Old"));
        }
    }

    // 강화형 OR 특수형 해금 시 보급형+예비용 계열 제거
    if (!bHasEnhanced && !bHasSpecial)
    {
        if (!PlayerChar->HasWeapon(EWeaponType::Supply))
        {
            FUpgradeData NewSupply;
            NewSupply.UpgradeType = EUpgradeType::NewWeapon;
            NewSupply.WeaponTarget = EWeaponType::Supply;
            NewSupply.CardTexture = LoadCardTexture(NewWeaponPathMap[EWeaponType::Supply]);
            Pool.Add(NewSupply);
            UE_LOG(LogTemp, Warning, TEXT("NewWeapon Added: Supply"));
        }
        if (!PlayerChar->HasWeapon(EWeaponType::Spare))
        {
            FUpgradeData NewSpare;
            NewSpare.UpgradeType = EUpgradeType::NewWeapon;
            NewSpare.WeaponTarget = EWeaponType::Spare;
            NewSpare.CardTexture = LoadCardTexture(NewWeaponPathMap[EWeaponType::Spare]);
            Pool.Add(NewSpare);
            UE_LOG(LogTemp, Warning, TEXT("NewWeapon Added: Spare"));
        }
    }

    // 조합 카드
    if (PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Standard) == 3 &&
        PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Old) == 3)
    {
        FUpgradeData Combine;
        Combine.UpgradeType = EUpgradeType::WeaponCombine;
        Combine.WeaponTarget = EWeaponType::Standard;
        Combine.WeaponTarget2 = EWeaponType::Old;
        Combine.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/gkqcp-Photoroom"));
        Pool.Add(Combine);
        UE_LOG(LogTemp, Warning, TEXT("Combine Added: Standard+Old->Improved"));
    }
    if (PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Supply) == 3 &&
        PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Spare) == 3)
    {
        FUpgradeData Combine;
        Combine.UpgradeType = EUpgradeType::WeaponCombine;
        Combine.WeaponTarget = EWeaponType::Supply;
        Combine.WeaponTarget2 = EWeaponType::Spare;
        Combine.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/gkqcp2-Photoroom"));
        Pool.Add(Combine);
        UE_LOG(LogTemp, Warning, TEXT("Combine Added: Supply+Spare->Enhanced"));
    }
    if (PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Enhanced) == 3 &&
        PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Improved) == 3)
    {
        FUpgradeData Combine;
        Combine.UpgradeType = EUpgradeType::WeaponCombine;
        Combine.WeaponTarget = EWeaponType::Enhanced;
        Combine.WeaponTarget2 = EWeaponType::Improved;
        Combine.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/gkacp3"));
        Pool.Add(Combine);
        UE_LOG(LogTemp, Warning, TEXT("Combine Added: Enhanced+Improved->Special"));
    }

    return Pool;
}

void ULevelUpWidget::SetupRandomCards()
{
    ASP_Character* PlayerChar = Cast<ASP_Character>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (!PlayerChar)
    {
        UE_LOG(LogTemp, Error, TEXT("SP_Character Cast Failed!"));
        return;
    }

    TArray<FUpgradeData> Pool = BuildUpgradePool(PlayerChar);
    UE_LOG(LogTemp, Warning, TEXT("Pools : %d"), Pool.Num());

    // 풀 셔플
    for (int32 i = Pool.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        Pool.Swap(i, j);
    }

    // 4개 뽑기
    CurrentUpgrades.Empty();
    for (int32 i = 0; i < 4 && i < Pool.Num(); i++)
    {
        CurrentUpgrades.Add(Pool[i]);
    }

    // 4개 미만이면 빈 카드 프레임으로 채우기
    UTexture2D* EmptyCardTexture = Cast<UTexture2D>(
        StaticLoadObject(UTexture2D::StaticClass(), nullptr,
            TEXT("/Game/UI/InGameUI/png/CardFrame")));

    while (CurrentUpgrades.Num() < 4)
    {
        FUpgradeData EmptyCard;
        EmptyCard.UpgradeType = EUpgradeType::MaxHP;
        EmptyCard.CardTexture = EmptyCardTexture;
        EmptyCard.WeaponTarget = EWeaponType::None;
        CurrentUpgrades.Add(EmptyCard);
    }

    for (int32 i = 0; i < CurrentUpgrades.Num(); i++)
    {
        ApplyCardData(i, CurrentUpgrades[i]);
    }
}

void ULevelUpWidget::ApplyCardData(int32 Index, const FUpgradeData& Data)
{
    UImage* ImgBlock = nullptr;
    switch (Index)
    {
    case 0: ImgBlock = CardImage1; break;
    case 1: ImgBlock = CardImage2; break;
    case 2: ImgBlock = CardImage3; break;
    case 3: ImgBlock = CardImage4; break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Card%d - Type:%d WeaponTarget:%d Texture:%s"),
        Index,
        (int32)Data.UpgradeType,
        (int32)Data.WeaponTarget,
        Data.CardTexture ? *Data.CardTexture->GetName() : TEXT("NULL"));

    if (ImgBlock && Data.CardTexture)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(Data.CardTexture);
        Brush.ImageSize = FVector2D(307.0f, 480.0f);
        ImgBlock->SetBrush(Brush);
    }
    else if (ImgBlock)
    {
        ImgBlock->SetBrush(FSlateBrush());
    }
}

void ULevelUpWidget::OnCard1Clicked() { OnCardSelected(0); }
void ULevelUpWidget::OnCard2Clicked() { OnCardSelected(1); }
void ULevelUpWidget::OnCard3Clicked() { OnCardSelected(2); }
void ULevelUpWidget::OnCard4Clicked() { OnCardSelected(3); }

void ULevelUpWidget::OnCardSelected(int32 Index)
{
    if (!CurrentUpgrades.IsValidIndex(Index)) return;

    FUpgradeData Selected = CurrentUpgrades[Index];

    // 빈 카드 클릭 시 무시
    if (Selected.WeaponTarget == EWeaponType::None &&
        Selected.CardTexture &&
        Selected.CardTexture->GetName() == TEXT("CardFrame"))
    {
        return;
    }

    ASP_Character* PlayerChar = Cast<ASP_Character>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (PlayerChar)
    {
        switch (Selected.UpgradeType)
        {
        case EUpgradeType::MaxHP:
            PlayerChar->MaxHealth += 20.0f;
            PlayerChar->CurrentHealth += 20.0f;
            PlayerChar->SyncHUDValues();
            break;
        case EUpgradeType::HPRecover:
            PlayerChar->CurrentHealth = FMath::Min(
                PlayerChar->CurrentHealth + 30.0f, PlayerChar->MaxHealth);
            PlayerChar->SyncHUDValues();
            break;
        case EUpgradeType::AttackUp:
            PlayerChar->AttackPower *= 1.1f;
            break;
        case EUpgradeType::WeaponEnhance:
            PlayerChar->EnhanceWeapon(Selected.WeaponTarget);
            break;
        case EUpgradeType::NewWeapon:
            PlayerChar->AddWeapon(Selected.WeaponTarget);
            break;
        case EUpgradeType::WeaponCombine:
            PlayerChar->CombineWeapons(Selected.WeaponTarget, Selected.WeaponTarget2);
            break;
        }
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        RemoveFromParent();
        PC->SetPause(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}