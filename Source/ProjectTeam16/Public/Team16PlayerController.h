// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "GameFramework/PlayerController.h"
#include "Team16PlayerController.generated.h"

class UInputAction;
class UAudioComponent;
class USoundBase;
class UClearResult;
class UGameOver;
class UInGameHUD;
class UUserWidget;
class ULevelUpWidget;
class UOptionWidget;
class UWidget;
class UImage;

UCLASS()
class PROJECTTEAM16_API ATeam16PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void PlayFadeOut();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void FadeToLevel(FName LevelName, const FString& Options);

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

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void RegisterDamageTaken(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOver();

	UFUNCTION(BlueprintCallable, Category = "UI")

	void StartBossClearSequence();

	void ShowLevelUpUI();
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseLevelUpUI();

	UFUNCTION(BlueprintCallable, Category = "UI|Timer")
	void SetGameTimerStartSeconds(int32 NewStartSeconds);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenOptionUI();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayMainMenuBGM();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayInGameBGM();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopBGM();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetBGMVolume(float NewVolume);
protected:
	bool UpdateFadeOut(float DeltaTime);
	bool UpdateFadeToLevel(float DeltaTime);
	bool UpdateBossClearAnimation(float DeltaTime);
	bool UpdateFadeToClearResult(float DeltaTime);

	void StartGameTimer();
	void StopGameTimer();
	void HandleGameTimerTick();
	void UpdateHUDTime();
	void ShowBossClearAnnouncement();
	void FadeToClearResult();
	void ShowClearResult();
	bool EnsureTakeDamageWidgetClass();
	void TriggerTakeDamageFeedback();
	void StartTakeDamageFeedback();
	bool UpdateTakeDamageFeedback(float DeltaTime);
	void SetTakeDamageOpacity(float Opacity) const;
	void StopTakeDamageFeedback();
	bool EnsureFadeScreenWidgetClass();
	void ApplyBossClearAnimation(float ElapsedTime) const;
	void ApplyWidgetAnimationState(UWidget* TargetWidget, float Opacity, const FVector2D& Scale, const FVector2D& Translation, float Angle = 0.0f) const;
	void PlayBGMByMode(bool bUseMainMenuBGM);
	void ResolveBGMSoundAssets();
	float GetCurrentBGMVolume() const;
	void ApplyGameOverBGMDuck();

	UFUNCTION()
	void HandleBGMFinished();

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
	TSubclassOf<UUserWidget> BossClearWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> BossClearWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UClearResult> ClearResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UClearResult> ClearResultWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UInGameHUD> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UInGameHUD> HUDWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Damage")
	TSubclassOf<UUserWidget> TakeDamageWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> TakeDamageWidgetInstance;

	UPROPERTY()
	TObjectPtr<UImage> TakeDamageBloodImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Damage", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float TakeDamagePeakOpacity = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Damage", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float TakeDamageFadeInDuration = 0.06f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Damage", meta = (ClampMin = "0.05", UIMin = "0.05"))
	float TakeDamageFadeOutDuration = 0.65f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Damage", meta = (ClampMin = "0.05", UIMin = "0.05"))
	float TakeDamageRetriggerInterval = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> MainMenuBGMSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> InGameBGMSound;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BGMComponent;

	bool bIsMainMenuBGMMode = false;
	bool bIsStoppingBGM = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float MasterBGMVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float MainMenuBGMVolume = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float InGameBGMVolume = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float GameOverBGMVolumeScale = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> FadeScreenWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float FadeOutDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float FadeToLevelDuration = 0.5f;

	UPROPERTY()
	TObjectPtr<UUserWidget> FadeScreenWidgetInstance;

	FTSTicker::FDelegateHandle FadeOutTickerHandle;
	float FadeOutElapsedTime = 0.0f;

	FTSTicker::FDelegateHandle FadeToLevelTickerHandle;
	float FadeToLevelElapsedTime = 0.0f;
	FName PendingFadeLevelName;
	FString PendingFadeLevelOptions;
	bool bIsFadingToLevel = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|BossClear", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BossClearStartDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|BossClear", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float BossClearDisplayDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|BossClear", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float FadeToClearResultDuration = 0.5f;

	FTimerHandle BossClearStartTimerHandle;
	FTimerHandle BossClearToResultTimerHandle;

	FTSTicker::FDelegateHandle BossClearAnimationTickerHandle;
	float BossClearAnimationElapsedTime = 0.0f;

	FTSTicker::FDelegateHandle FadeToClearResultTickerHandle;
	float FadeToClearResultElapsedTime = 0.0f;

	FTSTicker::FDelegateHandle TakeDamageTickerHandle;
	float TakeDamageElapsedTime = 0.0f;
	float LastTakeDamageTriggerTime = -1000.0f;
	bool bTakeDamageActive = false;
	bool bTakeDamageQueued = false;

	bool bBossClearSequenceStarted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 ZombieKillCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	float TotalDamageTaken = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Timer", meta = (ClampMin = "0", UIMin = "0"))
	int32 GameTimerStartSeconds = 600;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 GameTimerRemainingSeconds = 600;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 SurvivalElapsedSeconds = 0;

	FTimerHandle GameTimerHandle;
	bool bBossHUDActive = false;

	// 레벨업 UI (에디터에서 지정 가능)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULevelUpWidget> LevelUpWidgetClass;

private:

	//void CheatMaxEnhanceAllWeapons();

	UPROPERTY()
	ULevelUpWidget* LevelUpWidget;

	bool bIsLevelUpUIOpen = false;

	// 큐브 UI
	UPROPERTY()
	class UUserWidget* OptionWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> OptionWidgetClass;



	bool bIsOptionUIOpen = false;
public:
	UPROPERTY()
	TObjectPtr<UOptionWidget> OptionWidgetInstance;

	void OpenLevelUpUI();
};

