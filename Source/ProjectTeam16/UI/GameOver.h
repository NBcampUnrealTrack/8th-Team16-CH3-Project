// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOver.generated.h"

class UButton;
class UImage;
class UMaterialInterface;
class UMediaPlayer;
class UMediaSource;
class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UGameOver : public UUserWidget
{
	GENERATED_BODY()

public:
	UGameOver(const FObjectInitializer& ObjectInitializer);

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

	UFUNCTION()
	void HandleGameOverVideoOpened(FString OpenedUrl);

	UFUNCTION()
	void HandleGameOverVideoEndReached();

	void StartGameOverBackgroundVideo();
	void StopGameOverBackgroundVideo();
	void SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RetryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitGameButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GameOver;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> RetryButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> MainMenuButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> QuitGameButtonImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameOverText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SurvivalTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> RespawnProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background Video", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMediaPlayer> GameOverMediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background Video", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMediaSource> GameOverMediaSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background Video", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> GameOverVideoMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageNormalTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageHoveredTint = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
};
