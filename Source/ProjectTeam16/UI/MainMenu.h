// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class UButton;
class UImage;
class UMaterialInterface;
class UMediaPlayer;
class UMediaSource;
class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenu(const FObjectInitializer& ObjectInitializer);

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

	UFUNCTION()
	void HandleMainMenuVideoOpened(FString OpenedUrl);

	UFUNCTION()
	void HandleMainMenuVideoEndReached();

	void SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const;
	void StartMainMenuBackgroundVideo();
	void StopMainMenuBackgroundVideo();

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
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StartText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageNormalTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageHoveredTint = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background Video", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMediaPlayer> MainMenuMediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background Video", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMediaSource> MainMenuMediaSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background Video", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> MainMenuVideoMaterial;
};
