// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Blueprint/UserWidget.h"
#include "Slate/WidgetTransform.h"
#include "ClearResult.generated.h"

class UButton;
class UImage;
class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS()
class PROJECTTEAM16_API UClearResult : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Hide();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateScore(int32 KillCount);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateClearTime(int32 ClearSeconds);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateClearStats(int32 ClearSeconds, int32 KillCount, float DamageTaken);

protected:
	UFUNCTION()
	void OnRetryButtonClicked();

	UFUNCTION()
	void OnRetryButtonHovered();

	UFUNCTION()
	void OnRetryButtonUnhovered();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonHovered();

	UFUNCTION()
	void OnMainMenuButtonUnhovered();

	UFUNCTION()
	void OnQuitGameButtonClicked();

	UFUNCTION()
	void OnQuitGameButtonHovered();

	UFUNCTION()
	void OnQuitGameButtonUnhovered();

	FString GetRankForDamage(float DamageTaken) const;
	void SetButtonHovered(UButton* TargetButton, UImage* TargetImage, bool bIsHovered) const;
	void ResolveOptionalWidgets();
	void InitializeRevealState();
	void StartRevealSequence();
	void StopRevealSequence();
	bool UpdateRevealSequence(float DeltaTime);
	void ApplyWidgetReveal(UWidget* TargetWidget, float Alpha, ESlateVisibility VisibleState) const;

	struct FRevealItem
	{
		TWeakObjectPtr<UWidget> Widget;
		FWidgetTransform BaseTransform;
		float StartTime = 0.0f;
		bool bEnableOnFinish = false;
	};

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RetryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitGameButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> RetryButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> MainMenuButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> QuitButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClearTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SurvivalTimeValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KillCountValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DamageTakenValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RankValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KillCountValueText2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SurvivalTimeValueText2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DamageTakenValueText2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RankValueText2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GameOverImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GameOverImage2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ClearProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonNormalTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonHoveredTint = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float OverlayFadeOutDuration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float TitlePopDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float TitleShakeDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float TitleToStatsDelay = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StatItemInterval = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float StatPopDuration = 0.18f;

	FTSTicker::FDelegateHandle RevealTickerHandle;
	float RevealElapsedTime = 0.0f;
	TArray<FRevealItem> RevealItems;
	FWidgetTransform GameOverImageBaseTransform;
	FWidgetTransform GameOverImage2BaseTransform;
};
