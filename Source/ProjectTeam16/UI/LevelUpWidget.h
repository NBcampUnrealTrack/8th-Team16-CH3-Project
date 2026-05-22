#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeDataUI.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "LevelUpWidget.generated.h"

class ASP_Character;

UENUM(BlueprintType)
enum class ECardType : uint8
{
    None,
    Enhance_Left,   // 왼손 무기 강화
    Enhance_Right,  // 오른손 무기 강화
    New_Left,       // 왼손 신규 무기
    New_Right,      // 오른손 신규 무기
    Evolve_Left,    // 왼손 무기 진화 (조건 만족 시 등장)
    Evolve_Right,   // 오른손 무기 진화 (조건 만족 시 등장)

    // 💡 [신규] 스탯별 강화 타입을 세분화합니다.
    Upgrade_MaxHP,
    Upgrade_MaxStamina,
    Upgrade_AttackPower,
    HPRecover       // 기존 SpecialItem 대신 확정 체력 회복으로 활용 가능
};

USTRUCT()
struct FCardData
{
    GENERATED_BODY()

    ECardType CardType = ECardType::None;
    UTexture2D* CardTexture = nullptr;
    FString CardLabel;
};

UCLASS()
class PROJECTTEAM16_API ULevelUpWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    class UButton* CardButton1;

    UPROPERTY(meta = (BindWidget))
    class UButton* CardButton2;

    UPROPERTY(meta = (BindWidget))
    class UButton* CardButton3;

    UPROPERTY(meta = (BindWidget))
    class UImage* CardImage1;

    UPROPERTY(meta = (BindWidget))
    class UImage* CardImage2;

    UPROPERTY(meta = (BindWidget))
    class UImage* CardImage3;

    UFUNCTION(BlueprintCallable)
    void SetupRandomCards();

    UPROPERTY(BlueprintReadOnly, Category = "StatUpgrade")
    int32 MaxHealthLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "StatUpgrade")
    int32 MaxStaminaLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "StatUpgrade")
    int32 AttackPowerLevel = 0;

    const int32 MAX_UPGRADE_LEVEL = 8; // 최대 8강 정의

    // 애니메이션 레퍼런스
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* Anim_DimIn;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* Anim_CardDeal;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardTitle1;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardTitle2;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardTitle3;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardDesc1;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardDesc2;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardDesc3;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardLevel1;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardLevel2;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CardLevel3;

private:
    UTexture2D* LoadCardTexture(const FString& TexturePath);

    TArray<FUpgradeData> CurrentUpgrades;
    TArray<FUpgradeData> BuildUpgradePool(ASP_Character* PlayerChar);
    void ApplyCardData(int32 Index, const FUpgradeData& Data);

    UFUNCTION() void OnCard1Clicked();
    UFUNCTION() void OnCard2Clicked();
    UFUNCTION() void OnCard3Clicked();
    void OnCardSelected(int32 Index);
    void ApplyUpgradeAndClose(int32 Index);

    int32 PendingUpgradeIndex = 0;

    UFUNCTION()
    void OnCardDealAnimFinished();

    UFUNCTION()
    void OnCardDealInAnimFinished();

};