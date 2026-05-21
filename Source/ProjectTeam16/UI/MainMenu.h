// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "MainMenu.generated.h"

class UButton;
class UImage;
class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Hide();

protected:
	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnStartButtonHovered();

	UFUNCTION()
	void OnStartButtonUnhovered();

	UFUNCTION()
	void OnOptionsButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();

	UFUNCTION()
	void OnQuitButtonHovered();

	UFUNCTION()
	void OnQuitButtonUnhovered();

	void SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const;
	void ResolveBackgroundLightsOffImage();
	void StartMainMenuLightFlicker();
	void StopMainMenuLightFlicker();
	void ScheduleNextMainMenuLightFlicker();
	void ShowMainMenuLightsOffFrame();
	void HideMainMenuLightsOffFrame();
	bool UpdateMainMenuLightFlicker(float DeltaTime);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> OptionsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> StartButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> QuitButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BackgroundVideoImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BackgroundVideoImage_1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StartText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageNormalTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageHoveredTint = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Flicker", meta = (AllowPrivateAccess = "true"))
	bool bEnableMainMenuLightFlicker = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Flicker", meta = (ClampMin = "0.01", AllowPrivateAccess = "true"))
	float LightFlickerMinDelay = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Flicker", meta = (ClampMin = "0.01", AllowPrivateAccess = "true"))
	float LightFlickerMaxDelay = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Flicker", meta = (ClampMin = "0.01", AllowPrivateAccess = "true"))
	float LightFlickerMinDuration = 0.035f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Flicker", meta = (ClampMin = "0.01", AllowPrivateAccess = "true"))
	float LightFlickerMaxDuration = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Flicker", meta = (ClampMin = "0.0", ClampMax = "1.0", AllowPrivateAccess = "true"))
	float LightFlickerLightsOffOpacity = 1.0f;

	bool bIsMainMenuLightFlickerRunning = false;
	bool bIsLightsOffFrameVisible = false;
	float CurrentFlickerPhaseElapsed = 0.0f;
	float CurrentFlickerPhaseDuration = 0.0f;
	FTSTicker::FDelegateHandle MainMenuLightFlickerTickerHandle;
};
