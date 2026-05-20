#include "ProjectTeam16/UI/LevelUpWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "Team16PlayerController.h"

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

    // 1. 최대 체력 증가 (8강 미만일 때만 등장)
    if (PlayerChar->MaxHealthLevel < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::MaxHP;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/CurHP"));
        Pool.Add(Data);
    }

    // 2. 공격력 증가 (8강 미만일 때만 등장)
    if (PlayerChar->AttackPowerLevel < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::AttackUp;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/ATKUP"));
        Pool.Add(Data);
    }

    // 3. 최대 스태미나 증가 (8강 미만일 때만 등장)
    if (PlayerChar->MaxStaminaLevel < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::StaminaUp;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/stamina"));
        Pool.Add(Data);
    }

    // 4. 상시 체력 회복 (선택지 다양성용 - 항상 등장)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::HPRecover;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/HPGenerate"));
        Pool.Add(Data);
    }

    // 5. 기본 무기 강화 (가지고 있고, 8강 미만일 때만 등장)
    if (PlayerChar->HasWeapon(EWeaponType::Pistol) && PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Pistol) < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::WeaponEnhance;
        Data.WeaponTarget = EWeaponType::Pistol;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/pistol"));
        Pool.Add(Data);
    }
    if (PlayerChar->HasWeapon(EWeaponType::Shotgun) && PlayerChar->GetWeaponEnhanceLevel(EWeaponType::Shotgun) < PlayerChar->MAX_UPGRADE_LEVEL)
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::WeaponEnhance;
        Data.WeaponTarget = EWeaponType::Shotgun;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/shotgun"));
        Pool.Add(Data);
    }

    // 6. 조합(진화) 카드 등장 조건 체크
    if (PlayerChar->CanEvolvePistol())
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::WeaponCombine;
        Data.WeaponTarget = EWeaponType::Pistol;
        Data.WeaponTarget2 = EWeaponType::None;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/requiem"));
        Pool.Add(Data);
    }
    if (PlayerChar->CanEvolveShotgun())
    {
        FUpgradeData Data;
        Data.UpgradeType = EUpgradeType::WeaponCombine;
        Data.WeaponTarget = EWeaponType::Shotgun;
        Data.WeaponTarget2 = EWeaponType::None;
        Data.CardTexture = LoadCardTexture(TEXT("/Game/UI/InGameUI/png/blast"));
        Pool.Add(Data);
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
    UE_LOG(LogTemp, Warning, TEXT("Total Pool Size: %d"), Pool.Num());

    // 피셔-예이츠 셔플 알고리즘으로 풀 셔플
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
        StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/Game/UI/InGameUI/png/CardFrame")));

    while (CurrentUpgrades.Num() < 4)
    {
        FUpgradeData EmptyCard;
        // 💡 중요: 빈 카드는 오작동 방지를 위해 전용 상태(타입 미지정 등)나 명시적 플래그 설정 권장
        // 여기서는 WeaponCombine(기본값 아님)에 Target=None 및 특정 텍스처로 구분 유효화
        EmptyCard.UpgradeType = EUpgradeType::HPRecover; // 아무 스위치에나 걸려도 안전하거나 무시되도록 처리
        EmptyCard.CardTexture = EmptyCardTexture;
        EmptyCard.WeaponTarget = EWeaponType::None;
        EmptyCard.WeaponTarget2 = EWeaponType::None;
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
        Index, (int32)Data.UpgradeType, (int32)Data.WeaponTarget,
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

    // 💡 안전한 빈 카드 예외 처리: 텍스처 이름 검증 혹은 WeaponTarget 판정
    if (Selected.CardTexture && Selected.CardTexture->GetName() == TEXT("CardFrame"))
    {
        UE_LOG(LogTemp, Log, TEXT("Empty Card Slot Clicked. Ignored."));
        return;
    }

    ASP_Character* PlayerChar = Cast<ASP_Character>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (PlayerChar)
    {
        switch (Selected.UpgradeType)
        {

        case EUpgradeType::MaxHP:

            // 1. 최대 강화 수치(MAX_UPGRADE_LEVEL = 8) 제한 체크
            if (PlayerChar->MaxHealthLevel < PlayerChar->MAX_UPGRADE_LEVEL)
            {
                PlayerChar->MaxHealthLevel++; // 레벨 증가 (진화 조건 등에 활용)

                // 2. 실제 스태미나 스탯 증가 (값은 기획에 맞게 조절, 예: +20.0f)
                PlayerChar->MaxHealthLevel += 20.0f;
                PlayerChar->CurrentHealth += 20.0f; // 최대치가 늘어난 만큼 현재치도 보너스 회복

                // 3. 변경된 스탯을 PlayerController 및 HUD 위젯에 동기화
                PlayerChar->SyncHUDValues();

                UE_LOG(LogTemp, Log, TEXT("MaxHealth Up! Level: %d, MaxStamina: %f"),
                    PlayerChar->MaxHealthLevel, PlayerChar->MaxHealth);
            }
        case EUpgradeType::HPRecover:
            PlayerChar->CurrentHealth = FMath::Min(
                PlayerChar->CurrentHealth + 30.0f, PlayerChar->MaxHealth);
            PlayerChar->SyncHUDValues();
            break;

        case EUpgradeType::AttackUp:
            // 1. 최대 강화 수치(MAX_UPGRADE_LEVEL = 8) 제한 체크
            if (PlayerChar->AttackPowerLevel < PlayerChar->MAX_UPGRADE_LEVEL)
            {
                PlayerChar->AttackPowerLevel++; // 레벨 증가 (진화 조건 등에 활용)

                // 2. 실제 스태미나 스탯 증가 (값은 기획에 맞게 조절, 예: +20.0f)
                PlayerChar->AttackPower *= 1.1f;

                // 3. 변경된 스탯을 PlayerController 및 HUD 위젯에 동기화
                PlayerChar->SyncHUDValues();

                UE_LOG(LogTemp, Log, TEXT("AttackPower Up! Level: %d, AttackPower: %f"),
                    PlayerChar->AttackPowerLevel, PlayerChar->AttackPower);
            }
        case EUpgradeType::StaminaUp:
            // 1. 최대 강화 수치(MAX_UPGRADE_LEVEL = 8) 제한 체크
            if (PlayerChar->MaxStaminaLevel < PlayerChar->MAX_UPGRADE_LEVEL)
            {
                PlayerChar->MaxStaminaLevel++; // 레벨 증가 (진화 조건 등에 활용)

                // 2. 실제 스태미나 스탯 증가 (값은 기획에 맞게 조절, 예: +20.0f)
                PlayerChar->MaxStamina += 20.0f;
                PlayerChar->CurrentStamina += 20.0f; // 최대치가 늘어난 만큼 현재치도 보너스 회복

                // 3. 변경된 스탯을 PlayerController 및 HUD 위젯에 동기화
                PlayerChar->SyncHUDValues();

                UE_LOG(LogTemp, Log, TEXT("MaxStamina Up! Level: %d, MaxStamina: %f"),
                    PlayerChar->MaxStaminaLevel, PlayerChar->MaxStamina);
            }
        case EUpgradeType::WeaponEnhance:
            PlayerChar->EnhanceWeapon(Selected.WeaponTarget);
            break;

        case EUpgradeType::WeaponCombine:
            PlayerChar->CombineWeapons(Selected.WeaponTarget, Selected.WeaponTarget2);
            break;
        }
    }

    // UI 닫기 및 일시정지 해제 프로세스
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(PC))
        {
            Team16PlayerController->CloseLevelUpUI();
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