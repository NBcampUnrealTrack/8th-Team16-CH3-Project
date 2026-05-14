// Source/ProjectTeam16/Cube/OptionWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RandomOptionComponent.h"
#include "ProjectTeam16/Cube/OptionTypes.h"
#include "OptionWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class PROJECTTEAM16_API UOptionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GradeText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Line1;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Line2;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Line3;

    UPROPERTY(meta = (BindWidget))
    UButton* RollButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CubeCountText;

    UFUNCTION()
    void OnClickRoll();

    UFUNCTION()
    void OnClickClose();

    void RefreshUI();

    FString GetOptionString(const FOptionLine& Data);
protected:
    UPROPERTY()
    URandomOptionComponent* OptionComponent;
};