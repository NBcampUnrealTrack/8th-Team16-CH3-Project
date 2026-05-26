#include "ProjectTeam16/UI/LevelUpWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "Team16PlayerController.h"
#include "Components/TextBlock.h"

void ULevelUpWidget::NativeConstruct()
{
    Super::NativeConstruct(); // 부모 함수 호출 누락 방지

    if (CardButton1) CardButton1->OnClicked.AddDynamic(this, &ULevelUpWidget::OnCard1Clicked);
    if (CardButton2) CardButton2->OnClicked.AddDynamic(this, &ULevelUpWidget::OnCard2Clicked);
    if (CardButton3) CardButton3->OnClicked.AddDynamic(this, &ULevelUpWidget::OnCard3Clicked);

    // 카드 데이터 세팅 완료 후 애니메이션 재생
    if (Anim_DimIn)
        PlayAnimation(Anim_DimIn);

    // 딤 애니메이션 직후 카드 등장 (0.2초 딜레이)
    if (Anim_CardDeal)
        PlayAnimation(Anim_CardDeal, 0.2f);
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

    // 1. 최대 체력 증가 (8강 미만일 때만 등장)
    if (PlayerChar->MaxHealthLevel < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::MaxHP;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/CurHP"));
        Data.CardTitle = FText::FromString(TEXT("최대 체력 증가"));
        Data.CardDescription = FText::FromString(TEXT("최대 체력이 증가합니다."));
        Data.CardLevel = FText::FromString(
            FString::Printf(TEXT("%d / %d"), PlayerChar->MaxHealthLevel, PlayerChar->MAX_UPGRADE_LEVEL));
        Pool.Add(Data);
    }

    // 2. 공격력 증가 (8강 미만일 때만 등장)
    if (PlayerChar->AttackPowerLevel < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = ::EUpgradeType::AttackUp;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/ATKUP"));
        Data.CardTitle = FText::FromString(TEXT("공격력 증가"));
        Data.CardDescription = FText::FromString(TEXT("공격력이 증가합니다."));
        Data.CardLevel = FText::FromString(
            FString::Printf(TEXT("%d / %d"), PlayerChar->AttackPowerLevel, PlayerChar->MAX_UPGRADE_LEVEL));
        Pool.Add(Data);
    }

    // 3. 최대 스태미나 증가 (8강 미만일 때만 등장)
    if (PlayerChar->MaxStaminaLevel < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = ::EUpgradeType::StaminaUp;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/Stamina"));
        Data.CardTitle = FText::FromString(TEXT("최대 스태미나 증가"));
        Data.CardDescription = FText::FromString(TEXT("최대 스태미나가 증가합니다."));
        Data.CardLevel = FText::FromString(
            FString::Printf(TEXT("%d / %d"), PlayerChar->MaxStaminaLevel, PlayerChar->MAX_UPGRADE_LEVEL));
        Pool.Add(Data);
    }

    // 4. 상시 체력 회복 (항상 등장)
    {
        FUpgradeData Data;
        Data.UpgradeType = ::EUpgradeType::HPRecover;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/HPGenerate"));
        Data.CardTitle = FText::FromString(TEXT("체력 회복"));
        Data.CardDescription = FText::FromString(TEXT("체력을 회복합니다."));
        Data.CardLevel = FText::FromString(TEXT(""));  // 단계 없음
        Pool.Add(Data);
    }

    // 5. 기본 무기 강화 (가지고 있고, 8강 미만일 때만 등장)
    if (PlayerChar->HasWeapon(EWeaponType::Pistol) && PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Pistol) < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = ::EUpgradeType::WeaponEnhance;
        Data.WeaponTarget = EWeaponType::Pistol;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/PistolPlus"));
        Data.CardTitle = FText::FromString(TEXT("권총 강화"));
        Data.CardDescription = FText::FromString(TEXT("권총의 성능이 향상됩니다."));
        Data.CardLevel = FText::FromString(
            FString::Printf(TEXT("%d / %d"),
                PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Pistol), PlayerChar->MAX_UPGRADE_LEVEL));
        Pool.Add(Data);
    }

    if (PlayerChar->HasWeapon(EWeaponType::Shotgun) && PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Shotgun) < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = ::EUpgradeType::WeaponEnhance;
        Data.WeaponTarget = EWeaponType::Shotgun;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/ShotgunPlus"));
        Data.CardTitle = FText::FromString(TEXT("샷건 강화"));
        Data.CardDescription = FText::FromString(TEXT("샷건의 성능이 향상됩니다."));
        Data.CardLevel = FText::FromString(
            FString::Printf(TEXT("%d / %d"),
                PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Shotgun), PlayerChar->MAX_UPGRADE_LEVEL));
        Pool.Add(Data);
    }

    // 6. 조합(진화) 카드 등장 조건 체크
    if (PlayerChar->CanEvolvePistol())
    {
        FUpgradeData Data;
        Data.UpgradeType = ::EUpgradeType::WeaponCombine;
        Data.WeaponTarget = EWeaponType::Pistol;
        Data.WeaponTarget2 = EWeaponType::None;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/Requiem"));
        Data.CardTitle = FText::FromString(TEXT("레퀴엠"));
        Data.CardDescription = FText::FromString(TEXT("레퀴엠을 획득합니다."));
        Data.CardLevel = FText::FromString(TEXT("진화"));  // 강화 단계 대신 진화 표시
        Pool.Add(Data);
    }

    if (PlayerChar->CanEvolveShotgun())
    {
        FUpgradeData Data;
        Data.UpgradeType = ::EUpgradeType::WeaponCombine;
        Data.WeaponTarget = EWeaponType::Shotgun;
        Data.WeaponTarget2 = EWeaponType::None;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/Blast"));
        Data.CardTitle = FText::FromString(TEXT("블래스트"));
        Data.CardDescription = FText::FromString(TEXT("블래스트을 획득합니다."));
        Data.CardLevel = FText::FromString(TEXT("진화"));
        Pool.Add(Data);
    }

    // 7. 진화 무기 강화 — Requiem
    if (PlayerChar->HasEvolvedWeapon(EWeaponType::Requiem)
        && PlayerChar->GetEvolvedWeaponEnhanceLevel(EWeaponType::Requiem) < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::EvolvedWeaponEnhance;
        Data.WeaponTarget = EWeaponType::Requiem;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/RequiemPlus"));
        Data.CardTitle = FText::FromString(TEXT("레퀴엠 강화"));
        Data.CardDescription = FText::FromString(TEXT("레퀴엠의 성능이 향상됩니다."));
        Data.CardLevel = FText::FromString(
            FString::Printf(TEXT("%d / %d"),
                PlayerChar->GetEvolvedWeaponEnhanceLevel(EWeaponType::Requiem),
                PlayerChar->MAX_UPGRADE_LEVEL));
        Pool.Add(Data);
    }

    // 8. 진화 무기 강화 — Blast
    if (PlayerChar->HasEvolvedWeapon(EWeaponType::Blast)
        && PlayerChar->GetEvolvedWeaponEnhanceLevel(EWeaponType::Blast) < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::EvolvedWeaponEnhance;
        Data.WeaponTarget = EWeaponType::Blast;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/BlastPlus"));
        Data.CardTitle = FText::FromString(TEXT("블래스트 강화"));
        Data.CardDescription = FText::FromString(TEXT("블래스트의 성능이 향상됩니다."));
        Data.CardLevel = FText::FromString(
            FString::Printf(TEXT("%d / %d"),
                PlayerChar->GetEvolvedWeaponEnhanceLevel(EWeaponType::Blast),
                PlayerChar->MAX_UPGRADE_LEVEL));
        Pool.Add(Data);
    }

    for (int32 i = 0; i < Pool.Num(); i++)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pool[%d] Type: %d"), i, (int32)Pool[i].UpgradeType);
    }
    return Pool;
}

void ULevelUpWidget::SetupRandomCards()
{
    ASP_Character* PlayerChar = Cast<ASP_Character>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerChar) return;

    TArray<FUpgradeData> TotalPool = BuildUpgradePool(PlayerChar);

    // 피셔-예이츠 셔플 (중복 없이 뽑는 가장 안전한 방법)
    for (int32 i = TotalPool.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        TotalPool.Swap(i, j);
    }

    // 앞에서 3개만 사용
    CurrentUpgrades.Empty();
    int32 MaxCardsToShow = FMath::Min(3, TotalPool.Num());
    for (int32 i = 0; i < MaxCardsToShow; i++)
    {
        CurrentUpgrades.Add(TotalPool[i]);
    }

    for (int32 i = 0; i < 3; i++)
    {
        if (CurrentUpgrades.IsValidIndex(i))
            ApplyCardData(i, CurrentUpgrades[i]);
        else
            ApplyCardData(i, FUpgradeData());
    }

    // 버튼 비활성화 (애니메이션 끝나면 활성화)
    if (CardButton1) CardButton1->SetIsEnabled(false);
    if (CardButton2) CardButton2->SetIsEnabled(false);
    if (CardButton3) CardButton3->SetIsEnabled(false);

    if (Anim_DimIn) PlayAnimation(Anim_DimIn);
    if (Anim_CardDeal)
    {
        FWidgetAnimationDynamicEvent AnimFinished;
        AnimFinished.BindDynamic(this, &ULevelUpWidget::OnCardDealInAnimFinished);
        BindToAnimationFinished(Anim_CardDeal, AnimFinished);
        PlayAnimation(Anim_CardDeal, 0.2f);
    }
}

// 각각의 클릭 이벤트 매핑 함수들
void ULevelUpWidget::OnCard1Clicked() { OnCardSelected(0); }
void ULevelUpWidget::OnCard2Clicked() { OnCardSelected(1); }
void ULevelUpWidget::OnCard3Clicked() { OnCardSelected(2); }

void ULevelUpWidget::OnCardSelected(int32 Index)
{
    UE_LOG(LogTemp, Warning, TEXT("OnCardSelected Called: %d"), Index);

    if (!CurrentUpgrades.IsValidIndex(Index)) return;

    // 선택한 인덱스 저장
    PendingUpgradeIndex = Index;

    if (Anim_CardDeal)
    {
        // 역재생 끝났을 때 콜백으로 처리
        FWidgetAnimationDynamicEvent AnimFinished;
        AnimFinished.BindDynamic(this, &ULevelUpWidget::OnCardDealAnimFinished);
        BindToAnimationFinished(Anim_CardDeal, AnimFinished);

        PlayAnimationReverse(Anim_CardDeal);
        return;
    }

    ApplyUpgradeAndClose(Index);
}

void ULevelUpWidget::OnCardDealAnimFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("Anim Finished, Applying Upgrade"));
    ApplyUpgradeAndClose(PendingUpgradeIndex);
}

void ULevelUpWidget::ApplyUpgradeAndClose(int32 Index)
{
    if (!CurrentUpgrades.IsValidIndex(Index)) return;
    FUpgradeData Selected = CurrentUpgrades[Index];

    if (Selected.CardTexture && Selected.CardTexture->GetName() == TEXT("CardFrame"))
        return;

    ASP_Character* PlayerChar = Cast<ASP_Character>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (PlayerChar)
    {
        switch (Selected.UpgradeType)
        {
        case EUpgradeType::MaxHP:
            if (PlayerChar->MaxHealthLevel < PlayerChar->MAX_UPGRADE_LEVEL)
            {
                PlayerChar->MaxHealthLevel++;
                PlayerChar->MaxHealth += 20.0f;
                PlayerChar->CurrentHealth += 20.0f;
                PlayerChar->SyncHUDValues();
            }
            break;

        case EUpgradeType::HPRecover:
            PlayerChar->CurrentHealth = FMath::Min(
                PlayerChar->CurrentHealth + 30.0f, PlayerChar->MaxHealth);
            PlayerChar->SyncHUDValues();
            break;

        case EUpgradeType::AttackUp:
            if (PlayerChar->AttackPowerLevel < PlayerChar->MAX_UPGRADE_LEVEL)
            {
                PlayerChar->AttackPowerLevel++;
                PlayerChar->AttackPower *= 1.1f;
                PlayerChar->SyncHUDValues();
            }
            break;

        case EUpgradeType::StaminaUp:
            if (PlayerChar->MaxStaminaLevel < PlayerChar->MAX_UPGRADE_LEVEL)
            {
                PlayerChar->MaxStaminaLevel++;
                PlayerChar->MaxStamina += 20.0f;
                PlayerChar->CurrentStamina += 20.0f;
                PlayerChar->SyncHUDValues();
            }
            break;

        case EUpgradeType::WeaponEnhance:
            UE_LOG(LogTemp, Warning, TEXT("WeaponEnhance Called - WeaponTarget: %d"), (int32)Selected.WeaponTarget);
            PlayerChar->EnhanceWeapon(Selected.WeaponTarget);
            UE_LOG(LogTemp, Warning, TEXT("After EnhanceWeapon - Pistol Level: %d, Shotgun Level: %d"),
                PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Pistol),
                PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Shotgun));
            break;

        case EUpgradeType::WeaponCombine:
            PlayerChar->CombineWeapons(Selected.WeaponTarget, Selected.WeaponTarget2);
            break;
        case EUpgradeType::EvolvedWeaponEnhance:
            UE_LOG(LogTemp, Warning, TEXT("EvolvedWeaponEnhance Called - WeaponTarget: %d"),
                (int32)Selected.WeaponTarget);
            PlayerChar->EnhanceEvolvedWeapon(Selected.WeaponTarget);
            UE_LOG(LogTemp, Warning, TEXT("After EnhanceEvolvedWeapon - Pistol EvoLevel: %d, Shotgun EvoLevel: %d"),
                PlayerChar->GetEvolvedWeaponEnhanceLevel(EWeaponType::Pistol),
                PlayerChar->GetEvolvedWeaponEnhanceLevel(EWeaponType::Shotgun));
            break;
        }

        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            UE_LOG(LogTemp, Warning, TEXT("PC Valid, trying to close UI"));
            if (ATeam16PlayerController* Team16PC = Cast<ATeam16PlayerController>(PC))
            {
                UE_LOG(LogTemp, Warning, TEXT("CloseLevelUpUI Called"));
                Team16PC->CloseLevelUpUI();
            }
        }
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        if (ATeam16PlayerController* Team16PC = Cast<ATeam16PlayerController>(PC))
        {
            Team16PC->CloseLevelUpUI();
        }
        else
        {
            RemoveFromParent();
            PC->SetPause(false);
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }
    }
}

void ULevelUpWidget::ApplyCardData(int32 Index, const FUpgradeData& Data)
{
    UImage* ImgBlock = nullptr;
    UTextBlock* TitleBlock = nullptr;
    UTextBlock* DescBlock = nullptr;
    UTextBlock* LevelBlock = nullptr;

    switch (Index)
    {
    case 0: ImgBlock = CardImage1; TitleBlock = CardTitle1; DescBlock = CardDesc1; LevelBlock = CardLevel1; break;
    case 1: ImgBlock = CardImage2; TitleBlock = CardTitle2; DescBlock = CardDesc2; LevelBlock = CardLevel2; break;
    case 2: ImgBlock = CardImage3; TitleBlock = CardTitle3; DescBlock = CardDesc3; LevelBlock = CardLevel3; break;
    }

    if (ImgBlock && Data.CardTexture)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(Data.CardTexture);
        Brush.ImageSize = FVector2D(200.0f, 200.0f);
        ImgBlock->SetBrush(Brush);
    }

    // 텍스트
    if (TitleBlock) TitleBlock->SetText(Data.CardTitle);
    if (DescBlock)  DescBlock->SetText(Data.CardDescription);
    if (LevelBlock) LevelBlock->SetText(Data.CardLevel);
}

void ULevelUpWidget::OnCardDealInAnimFinished()
{
    // 등장 완료 → 버튼 활성화
    if (CardButton1) CardButton1->SetIsEnabled(true);
    if (CardButton2) CardButton2->SetIsEnabled(true);
    if (CardButton3) CardButton3->SetIsEnabled(true);

    // 콜백 해제 (중요! 안하면 퇴장 애니메이션 끝날 때도 여기 또 호출됨)
    UnbindAllFromAnimationFinished(Anim_CardDeal);
}