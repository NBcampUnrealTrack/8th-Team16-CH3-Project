#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeDataUI.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "LevelUpWidget.generated.h"

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
    class UButton* CardButton4;

    UPROPERTY(meta = (BindWidget))
    class UImage* CardImage1;

    UPROPERTY(meta = (BindWidget))
    class UImage* CardImage2;

    UPROPERTY(meta = (BindWidget))
    class UImage* CardImage3;

    UPROPERTY(meta = (BindWidget))
    class UImage* CardImage4;

    //UFUNCTION(BlueprintCallable)
    //void SetupRandomCards();

private:
    UTexture2D* LoadCardTexture(const FString& TexturePath);

    TArray<FUpgradeData> CurrentUpgrades;
    //TArray<FUpgradeData> BuildUpgradePool(ASP_Character* PlayerChar);
    void ApplyCardData(int32 Index, const FUpgradeData& Data);

    //UFUNCTION() void OnCard1Clicked();
    //UFUNCTION() void OnCard2Clicked();
    //UFUNCTION() void OnCard3Clicked();
    //UFUNCTION() void OnCard4Clicked();
    //void OnCardSelected(int32 Index);
    
};