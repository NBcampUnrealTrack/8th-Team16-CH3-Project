// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameHUD.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UInGameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Hide();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateStamina(float CurrentStamina, float MaxStamina);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateExperience(float CurrentExp, float MaxExp);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateLevel(int32 CurrentLevel);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateZombieKillCount(int32 KillCount);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateTime(int32 RemainingSeconds);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideTime();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowBossHealth(const FString& BossName, float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideBossHealth();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateBossHealth(float CurrentHealth, float MaxHealth);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ExperienceProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> StaminaProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HpText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ZombieKillCountText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHealthProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHealthProgressBar_1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossnameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossHealthText;
};
