// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

class UButton;
class UImage;
class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Hide();

protected:
	UFUNCTION()
	void OnResumeButtonClicked();

	UFUNCTION()
	void OnResumeButtonHovered();

	UFUNCTION()
	void OnResumeButtonUnhovered();

	UFUNCTION()
	void OnRestartButtonClicked();

	UFUNCTION()
	void OnRestartButtonHovered();

	UFUNCTION()
	void OnRestartButtonUnhovered();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonHovered();

	UFUNCTION()
	void OnMainMenuButtonUnhovered();

	UFUNCTION()
	void OnQuitButtonClicked();

	UFUNCTION()
	void OnQuitButtonHovered();

	UFUNCTION()
	void OnQuitButtonUnhovered();

	void SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ResumeImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> RestartImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> MainMenuImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> QuitImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PauseTitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> VolumeProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageNormalTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	FLinearColor ButtonImageHoveredTint = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
};
