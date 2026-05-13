// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Team16PlayerController.generated.h"

class UInputAction;
class UGameOver;
class UInGameHUD;
class UUserWidget;
class ULevelUpWidget;

UCLASS()
class PROJECTTEAM16_API ATeam16PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInGameHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideInGameHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUDHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUDStamina(float CurrentStamina, float MaxStamina);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUDExperience(float CurrentExp, float MaxExp);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUDLevel(int32 CurrentLevel);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUDZombieKillCount(int32 KillCount);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHUDBossHealth(const FString& BossName, float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUDBossHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideHUDBossHealth();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void RegisterZombieKill(int32 ExpReward);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOver();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLevelUpUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseLevelUpUI();

	UFUNCTION(BlueprintCallable, Category = "UI|Timer")
	void SetGameTimerStartSeconds(int32 NewStartSeconds);

protected:
	void StartGameTimer();
	void StopGameTimer();
	void HandleGameTimerTick();
	void UpdateHUDTime();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UGameOver> GameOverWidgetClass;

	UPROPERTY()
	TObjectPtr<UGameOver> GameOverWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UInGameHUD> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UInGameHUD> HUDWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 ZombieKillCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Timer", meta = (ClampMin = "0", UIMin = "0"))
	int32 GameTimerStartSeconds = 600;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 GameTimerRemainingSeconds = 600;

	FTimerHandle GameTimerHandle;

	// 레벨업 UI (에디터에서 지정 가능)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULevelUpWidget> LevelUpWidgetClass;

private:
	void OpenLevelUpUI();
	void CheatMaxEnhanceAllWeapons();

	UPROPERTY()
	ULevelUpWidget* LevelUpWidget;

	bool bIsLevelUpUIOpen = false;
};

