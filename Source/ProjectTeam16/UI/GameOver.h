// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "GameOver.generated.h"

class UBorder;
class UButton;
class UImage;
class UTextBlock;
class UWidget;

UCLASS()
class PROJECTTEAM16_API UGameOver : public UUserWidget
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
	void UpdateSurvivalTime(int32 SurvivalSeconds);

protected:
	UFUNCTION()
	void OnMainMenuButtonClicked();

	void InitializeRevealState();
	void StartRevealSequence();
	void StopRevealSequence();
	bool UpdateRevealSequence(float DeltaTime);
	void ApplyWidgetReveal(UWidget* TargetWidget, float Alpha, ESlateVisibility VisibleState) const;
	void ResolveOptionalWidgets();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GameOverImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GameOverImage2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> KillcountImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> TimeImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Killcount;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Killcount2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Time;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Time2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float RedFadeDuration = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float GameOverImagesFadeDuration = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float StatsFadeDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float MainMenuButtonFadeDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StageGapDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float RedOverlayTargetOpacity = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal", meta = (AllowPrivateAccess = "true"))
	FLinearColor RedOverlayColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	FTSTicker::FDelegateHandle RevealTickerHandle;
	float RevealElapsedTime = 0.0f;
	int32 CachedKillCount = 0;
	int32 CachedSurvivalSeconds = 0;
};
