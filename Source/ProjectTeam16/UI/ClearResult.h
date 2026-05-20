// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClearResult.generated.h"

class UButton;
class UImage;
class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UClearResult : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

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
	void OnContinueButtonClicked();

	UFUNCTION()
	void OnContinueButtonHovered();

	UFUNCTION()
	void OnContinueButtonUnhovered();

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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ContinueButton;

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
	TObjectPtr<UProgressBar> ClearProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonNormalTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonHoveredTint = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
};
