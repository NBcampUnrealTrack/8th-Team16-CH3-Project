// Source/ProjectTeam16/Cube/OptionWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RandomOptionComponent.h"
#include "ProjectTeam16/Cube/OptionTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
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

    // 애니메이션 바인딩 추가
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* Anim_FlipOut;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* Anim_FlipIn;

    // 애니메이션 중 잠금 플래그
    bool bIsAnimating = false;

    // 콜백 함수
    UFUNCTION()
    void OnFlipOutFinished();

    UFUNCTION()
    void OnFlipInFinished();

    // 등급별 배경/테두리 이미지
    UPROPERTY(meta = (BindWidget))
    class UImage* GradeBorderImage;

    UPROPERTY(meta = (BindWidget))
    class UImage* GradeBackgroundImage;

    // 기존 GradeBackgroundImage 대신 등급별 텍스처
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* RareBorderTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* EpicBorderTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* UniqueBorderTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* LegendaryBorderTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* RareBackgroundTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* EpicBackgroundTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* UniqueBackgroundTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GradeVisual")
    UTexture2D* LegendaryBackgroundTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* RareSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* EpicSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* UniqueSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* LegendarySound;

    // 등급 업그레이드 시 파티클
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* EpicUpgradeVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* UniqueUpgradeVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* LegendaryUpgradeVFX;

    // 이전 등급 저장 (업그레이드 감지용)
    EOptionGrade PreviousGrade = EOptionGrade::Rare;

    // 등급별 시각 효과 적용 함수
    void ApplyGradeVisuals(EOptionGrade Grade, bool bJustUpgraded);

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* Anim_GradeUpgrade;
protected:
    UPROPERTY()
    URandomOptionComponent* OptionComponent;

    UPROPERTY()
    class UNiagaraComponent* ActiveUpgradeVFXComponent;
};