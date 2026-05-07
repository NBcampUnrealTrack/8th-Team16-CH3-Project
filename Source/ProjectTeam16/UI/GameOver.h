// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOver.generated.h"

class UButton;
class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UGameOver : public UUserWidget
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

protected:
	UFUNCTION()
	void OnRetryButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void OnQuitGameButtonClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RetryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitGameButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameOverText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> RespawnProgressBar;
};
