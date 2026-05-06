// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

class UButton;
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
	void OnRestartButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PauseTitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> VolumeProgressBar;
};
