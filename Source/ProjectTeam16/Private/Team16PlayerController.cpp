// Fill out your copyright notice in the Description page of Project Settings.

#include "Team16PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/UI/ClearResult.h"
#include "ProjectTeam16/UI/GameOver.h"
#include "ProjectTeam16/UI/InGameHUD.h"
#include "ProjectTeam16/UI/LevelUpWidget.h"
#include "ProjectTeam16/Cube/OptionWidget.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "Components/AudioComponent.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "Sound/SoundBase.h"

void ATeam16PlayerController::BeginPlay()
{
	Super::BeginPlay();

	ResolveBGMSoundAssets();
	PlayFadeOut();
}

void ATeam16PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent || !PauseAction)
	{
		return;
	}

	PauseAction->bTriggerWhenPaused = true;
	EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ATeam16PlayerController::TogglePauseMenu);
	
	InputComponent->BindAction("OpenLevelUp", IE_Pressed,
		this, &ATeam16PlayerController::OpenLevelUpUI);
	InputComponent->BindAction("OpenOption", IE_Pressed,
		this, &ATeam16PlayerController::OpenOptionUI);
}

void ATeam16PlayerController::TogglePauseMenu()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const bool bPauseMenuOpen = PauseMenuWidgetInstance && PauseMenuWidgetInstance->IsInViewport();
	if (bPauseMenuOpen)
	{
		PauseMenuWidgetInstance->RemoveFromParent();
		PauseMenuWidgetInstance = nullptr;

		UGameplayStatics::SetGamePaused(World, false);

		bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		return;
	}

	if (UGameplayStatics::IsGamePaused(World))
	{
		return;
	}

	if (!PauseMenuWidgetClass)
	{
		return;
	}

	if (!PauseMenuWidgetInstance)
	{
		PauseMenuWidgetInstance = CreateWidget<UUserWidget>(this, PauseMenuWidgetClass);
	}

	if (!PauseMenuWidgetInstance)
	{
		return;
	}

	if (!PauseMenuWidgetInstance->IsInViewport())
	{
		PauseMenuWidgetInstance->AddToViewport();
	}
	PauseMenuWidgetInstance->SetKeyboardFocus();

	UGameplayStatics::SetGamePaused(World, true);

	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ATeam16PlayerController::PlayFadeOut()
{
	if (!EnsureFadeScreenWidgetClass())
	{
		return;
	}

	if (FadeScreenWidgetInstance)
	{
		FadeScreenWidgetInstance->RemoveFromParent();
		FadeScreenWidgetInstance = nullptr;
	}

	FadeScreenWidgetInstance = CreateWidget<UUserWidget>(this, FadeScreenWidgetClass);
	if (!FadeScreenWidgetInstance)
	{
		return;
	}

	FadeScreenWidgetInstance->SetRenderOpacity(1.0f);
	FadeScreenWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
	FadeScreenWidgetInstance->AddToViewport(1000);

	if (FadeOutTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FadeOutTickerHandle);
		FadeOutTickerHandle.Reset();
	}

	FadeOutElapsedTime = 0.0f;
	FadeOutTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ATeam16PlayerController::UpdateFadeOut)
	);
}

bool ATeam16PlayerController::UpdateFadeOut(float DeltaTime)
{
	FadeOutElapsedTime += DeltaTime;

	const float FadeAlpha = 1.0f - FMath::Clamp(FadeOutElapsedTime / FadeOutDuration, 0.0f, 1.0f);
	if (FadeScreenWidgetInstance)
	{
		FadeScreenWidgetInstance->SetRenderOpacity(FadeAlpha);
	}

	if (FadeAlpha > 0.0f)
	{
		return true;
	}

	if (FadeScreenWidgetInstance)
	{
		FadeScreenWidgetInstance->RemoveFromParent();
		FadeScreenWidgetInstance = nullptr;
	}

	FadeOutTickerHandle.Reset();
	return false;
}

void ATeam16PlayerController::FadeToLevel(FName LevelName, const FString& Options)
{
	if (bIsFadingToLevel || LevelName.IsNone())
	{
		return;
	}

	if (!EnsureFadeScreenWidgetClass())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UGameplayStatics::SetGamePaused(World, false);
			UGameplayStatics::OpenLevel(World, LevelName, true, Options);
		}
		return;
	}

	if (FadeScreenWidgetInstance)
	{
		FadeScreenWidgetInstance->RemoveFromParent();
		FadeScreenWidgetInstance = nullptr;
	}

	FadeScreenWidgetInstance = CreateWidget<UUserWidget>(this, FadeScreenWidgetClass);
	if (!FadeScreenWidgetInstance)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UGameplayStatics::SetGamePaused(World, false);
			UGameplayStatics::OpenLevel(World, LevelName, true, Options);
		}
		return;
	}

	PendingFadeLevelName = LevelName;
	PendingFadeLevelOptions = Options;
	FadeToLevelElapsedTime = 0.0f;
	bIsFadingToLevel = true;

	FadeScreenWidgetInstance->SetRenderOpacity(0.0f);
	FadeScreenWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	FadeScreenWidgetInstance->AddToViewport(1000);

	if (FadeOutTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FadeOutTickerHandle);
		FadeOutTickerHandle.Reset();
	}

	if (FadeToLevelTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FadeToLevelTickerHandle);
		FadeToLevelTickerHandle.Reset();
	}

	FadeToLevelTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ATeam16PlayerController::UpdateFadeToLevel)
	);
}

bool ATeam16PlayerController::UpdateFadeToLevel(float DeltaTime)
{
	FadeToLevelElapsedTime += DeltaTime;

	const float FadeAlpha = FMath::Clamp(FadeToLevelElapsedTime / FadeToLevelDuration, 0.0f, 1.0f);
	if (FadeScreenWidgetInstance)
	{
		FadeScreenWidgetInstance->SetRenderOpacity(FadeAlpha);
	}

	if (FadeAlpha < 1.0f)
	{
		return true;
	}

	FadeToLevelTickerHandle.Reset();
	bIsFadingToLevel = false;

	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::SetGamePaused(World, false);
		UGameplayStatics::OpenLevel(World, PendingFadeLevelName, true, PendingFadeLevelOptions);
	}

	return false;
}

void ATeam16PlayerController::ShowInGameHUD()
{
	if (!HUDWidgetClass)
	{
		return;
	}

	if (!HUDWidgetInstance)
	{
		HUDWidgetInstance = CreateWidget<UInGameHUD>(this, HUDWidgetClass);
	}

	if (!HUDWidgetInstance)
	{
		return;
	}

	if (!HUDWidgetInstance->IsInViewport())
	{
		HUDWidgetInstance->AddToViewport();
	}

	HUDWidgetInstance->Show();
	HUDWidgetInstance->HideBossHealth();
	HUDWidgetInstance->ShowMiniMap();
	TotalDamageTaken = 0.0f;

	
	if (ASP_Character* PlayerCharacter = Cast<ASP_Character>(GetPawn()))
	{
		PlayerCharacter->SyncHUDValues();
	}
	HUDWidgetInstance->UpdateZombieKillCount(ZombieKillCount);
	PlayInGameBGM();

	StartGameTimer();
}

void ATeam16PlayerController::HideInGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->Hide();
	}

	StopTakeDamageFeedback();
	StopGameTimer();
	StopBGM();
}

void ATeam16PlayerController::UpdateHUDHealth(float CurrentHealth, float MaxHealth)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateHealth(CurrentHealth, MaxHealth);
	}
}

void ATeam16PlayerController::UpdateHUDStamina(float CurrentStamina, float MaxStamina)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateStamina(CurrentStamina, MaxStamina);
	}
}

void ATeam16PlayerController::UpdateHUDExperience(float CurrentExp, float MaxExp)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateExperience(CurrentExp, MaxExp);
	}
}

void ATeam16PlayerController::UpdateHUDLevel(int32 CurrentLevel)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateLevel(CurrentLevel);
	}
}

void ATeam16PlayerController::UpdateHUDZombieKillCount(int32 KillCount)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateZombieKillCount(KillCount);
	}
}

void ATeam16PlayerController::ShowHUDBossHealth(const FString& BossName, float CurrentHealth, float MaxHealth)
{
	bBossHUDActive = true;

	if (!HUDWidgetInstance && HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UInGameHUD>(this, HUDWidgetClass);
	}

	if (HUDWidgetInstance)
	{
		if (!HUDWidgetInstance->IsInViewport())
		{
			HUDWidgetInstance->AddToViewport();
		}

		HUDWidgetInstance->Show();
		HUDWidgetInstance->HideTime();
		HUDWidgetInstance->HideMiniMap();
		HUDWidgetInstance->ShowBossHealth(BossName, CurrentHealth, MaxHealth);
	}
}

void ATeam16PlayerController::UpdateHUDBossHealth(float CurrentHealth, float MaxHealth)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateBossHealth(CurrentHealth, MaxHealth);
	}
}

void ATeam16PlayerController::HideHUDBossHealth()
{
	bBossHUDActive = false;

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->HideBossHealth();
	}
}

void ATeam16PlayerController::RegisterZombieKill(int32 ExpReward)
{
	ZombieKillCount++;
	UpdateHUDZombieKillCount(ZombieKillCount);
	UE_LOG(LogTemp, Log, TEXT("Zombie kill registered. KillCount=%d ExpReward=%d"), ZombieKillCount, ExpReward);

	
	if (ASP_Character* PlayerCharacter = Cast<ASP_Character>(GetPawn()))
	{
		PlayerCharacter->AddExperience(ExpReward);

		PlayerCharacter->AddCube(1);
	}
}

void ATeam16PlayerController::RegisterDamageTaken(float DamageAmount)
{
	if (DamageAmount <= 0.0f)
	{
		return;
	}

	TotalDamageTaken += DamageAmount;
	TriggerTakeDamageFeedback();
}

void ATeam16PlayerController::ShowGameOver()
{
	UWorld* World = GetWorld();
	if (!World || !GameOverWidgetClass)
	{
		return;
	}

	HideInGameHUD();
	StopBGM();

	if (PauseMenuWidgetInstance)
	{
		PauseMenuWidgetInstance->RemoveFromParent();
		PauseMenuWidgetInstance = nullptr;
	}

	if (!GameOverWidgetInstance)
	{
		GameOverWidgetInstance = CreateWidget<UGameOver>(this, GameOverWidgetClass);
	}

	if (!GameOverWidgetInstance)
	{
		return;
	}

	if (!GameOverWidgetInstance->IsInViewport())
	{
		GameOverWidgetInstance->AddToViewport();
	}

	GameOverWidgetInstance->Show();
	GameOverWidgetInstance->UpdateScore(ZombieKillCount);
	GameOverWidgetInstance->UpdateSurvivalTime(FMath::Max(0, SurvivalElapsedSeconds));

	UGameplayStatics::SetGamePaused(World, true);

	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GameOverWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ATeam16PlayerController::StartBossClearSequence()
{
	if (bBossClearSequenceStarted)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bBossClearSequenceStarted = true;
	StopGameTimer();

	// 보스가 죽은 직후 바로 결과창을 띄우지 않고, 전투 마무리 연출을 위해 잠깐 대기합니다.
	World->GetTimerManager().ClearTimer(BossClearStartTimerHandle);
	World->GetTimerManager().SetTimer(
		BossClearStartTimerHandle,
		this,
		&ATeam16PlayerController::ShowBossClearAnnouncement,
		BossClearStartDelay,
		false
	);
}

void ATeam16PlayerController::ShowBossClearAnnouncement()
{
	if (!BossClearWidgetClass)
	{
		BossClearWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/HUD/WBP_BossClear.WBP_BossClear_C"));
	}

	if (!BossClearWidgetClass)
	{
		FadeToClearResult();
		return;
	}

	if (BossClearWidgetInstance)
	{
		BossClearWidgetInstance->RemoveFromParent();
		BossClearWidgetInstance = nullptr;
	}

	BossClearWidgetInstance = CreateWidget<UUserWidget>(this, BossClearWidgetClass);
	if (!BossClearWidgetInstance)
	{
		FadeToClearResult();
		return;
	}

	HideInGameHUD();
	BossClearWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
	BossClearWidgetInstance->AddToViewport(900);

	BossClearAnimationElapsedTime = 0.0f;
	ApplyBossClearAnimation(BossClearAnimationElapsedTime);

	if (BossClearAnimationTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BossClearAnimationTickerHandle);
		BossClearAnimationTickerHandle.Reset();
	}

	BossClearAnimationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ATeam16PlayerController::UpdateBossClearAnimation)
	);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BossClearToResultTimerHandle);
		World->GetTimerManager().SetTimer(
			BossClearToResultTimerHandle,
			this,
			&ATeam16PlayerController::FadeToClearResult,
			BossClearDisplayDuration,
			false
		);
	}
}

bool ATeam16PlayerController::UpdateBossClearAnimation(float DeltaTime)
{
	if (!BossClearWidgetInstance)
	{
		BossClearAnimationTickerHandle.Reset();
		return false;
	}

	BossClearAnimationElapsedTime += DeltaTime;
	ApplyBossClearAnimation(BossClearAnimationElapsedTime);
	return true;
}

void ATeam16PlayerController::ApplyBossClearAnimation(float ElapsedTime) const
{
	if (!BossClearWidgetInstance)
	{
		return;
	}

	UWidget* DarkOverlay = BossClearWidgetInstance->GetWidgetFromName(TEXT("DarkOverlay"));
	UWidget* CenterLineGlow = BossClearWidgetInstance->GetWidgetFromName(TEXT("CenterLineGlow"));
	UWidget* BossClearImage = BossClearWidgetInstance->GetWidgetFromName(TEXT("BossEliminatedImage"));
	if (!BossClearImage)
	{
		BossClearImage = BossClearWidgetInstance->GetWidgetFromName(TEXT("BossClearImage"));
	}

	const float DarkOpacity = FMath::Clamp(ElapsedTime / 0.45f, 0.0f, 1.0f) * 0.82f;
	ApplyWidgetAnimationState(DarkOverlay, DarkOpacity, FVector2D(1.0f, 1.0f), FVector2D::ZeroVector);

	const float GlowInAlpha = FMath::Clamp(ElapsedTime / 0.25f, 0.0f, 1.0f);
	const float GlowOutAlpha = 1.0f - FMath::Clamp((ElapsedTime - 0.35f) / 0.7f, 0.0f, 1.0f);
	const float GlowOpacity = GlowInAlpha * GlowOutAlpha;
	const float GlowScaleX = FMath::Lerp(0.2f, 1.35f, GlowInAlpha);
	ApplyWidgetAnimationState(CenterLineGlow, GlowOpacity, FVector2D(GlowScaleX, 1.0f), FVector2D::ZeroVector);

	const float ImageElapsedTime = FMath::Max(0.0f, ElapsedTime - 0.35f);
	const float ImageOpacity = FMath::Clamp(ImageElapsedTime / 0.2f, 0.0f, 1.0f);
	float ImageScale = 0.9f;
	if (ImageElapsedTime < 0.18f)
	{
		ImageScale = FMath::Lerp(0.9f, 1.12f, ImageElapsedTime / 0.18f);
	}
	else if (ImageElapsedTime < 0.38f)
	{
		ImageScale = FMath::Lerp(1.12f, 1.0f, (ImageElapsedTime - 0.18f) / 0.2f);
	}
	else
	{
		ImageScale = 1.0f;
	}

	const float ShakeAlpha = 1.0f - FMath::Clamp((ImageElapsedTime - 0.25f) / 1.1f, 0.0f, 1.0f);
	const float ShakeX = FMath::Sin(ImageElapsedTime * 55.0f) * 7.0f * ShakeAlpha;
	const float ShakeY = FMath::Cos(ImageElapsedTime * 47.0f) * 3.0f * ShakeAlpha;
	const float ShakeAngle = FMath::Sin(ImageElapsedTime * 42.0f) * 1.2f * ShakeAlpha;
	ApplyWidgetAnimationState(BossClearImage, ImageOpacity, FVector2D(ImageScale, ImageScale), FVector2D(ShakeX, ShakeY), ShakeAngle);
}

void ATeam16PlayerController::ApplyWidgetAnimationState(UWidget* TargetWidget, float Opacity, const FVector2D& Scale, const FVector2D& Translation, float Angle) const
{
	if (!TargetWidget)
	{
		return;
	}

	FWidgetTransform WidgetTransform;
	WidgetTransform.Translation = Translation;
	WidgetTransform.Scale = Scale;
	WidgetTransform.Angle = Angle;

	TargetWidget->SetRenderOpacity(Opacity);
	TargetWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	TargetWidget->SetRenderTransform(WidgetTransform);
}

void ATeam16PlayerController::FadeToClearResult()
{
	if (!EnsureFadeScreenWidgetClass())
	{
		ShowClearResult();
		return;
	}

	if (FadeScreenWidgetInstance)
	{
		FadeScreenWidgetInstance->RemoveFromParent();
		FadeScreenWidgetInstance = nullptr;
	}

	FadeScreenWidgetInstance = CreateWidget<UUserWidget>(this, FadeScreenWidgetClass);
	if (!FadeScreenWidgetInstance)
	{
		ShowClearResult();
		return;
	}

	FadeToClearResultElapsedTime = 0.0f;
	FadeScreenWidgetInstance->SetRenderOpacity(0.0f);
	FadeScreenWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	FadeScreenWidgetInstance->AddToViewport(1000);

	if (FadeToClearResultTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FadeToClearResultTickerHandle);
		FadeToClearResultTickerHandle.Reset();
	}

	FadeToClearResultTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ATeam16PlayerController::UpdateFadeToClearResult)
	);
}

bool ATeam16PlayerController::UpdateFadeToClearResult(float DeltaTime)
{
	FadeToClearResultElapsedTime += DeltaTime;

	const float FadeAlpha = FMath::Clamp(FadeToClearResultElapsedTime / FadeToClearResultDuration, 0.0f, 1.0f);
	if (FadeScreenWidgetInstance)
	{
		FadeScreenWidgetInstance->SetRenderOpacity(FadeAlpha);
	}

	if (FadeAlpha < 1.0f)
	{
		return true;
	}

	if (BossClearAnimationTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BossClearAnimationTickerHandle);
		BossClearAnimationTickerHandle.Reset();
	}

	if (BossClearWidgetInstance)
	{
		BossClearWidgetInstance->RemoveFromParent();
		BossClearWidgetInstance = nullptr;
	}

	ShowClearResult();
	PlayFadeOut();

	FadeToClearResultTickerHandle.Reset();
	return false;
}

bool ATeam16PlayerController::EnsureFadeScreenWidgetClass()
{
	if (FadeScreenWidgetClass)
	{
		return true;
	}

	FadeScreenWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/HUD/WBP_FadeScreen.WBP_FadeScreen_C"));
	if (!FadeScreenWidgetClass)
	{
		FadeScreenWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/UIAnimation/WBP_FadeScreen.WBP_FadeScreen_C"));
	}

	return FadeScreenWidgetClass != nullptr;
}

bool ATeam16PlayerController::EnsureTakeDamageWidgetClass()
{
	if (TakeDamageWidgetClass)
	{
		return true;
	}

	TakeDamageWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/HUD/WBP_TakeDamage.WBP_TakeDamage_C"));
	if (!TakeDamageWidgetClass)
	{
		TakeDamageWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/WBP_TakeDamage.WBP_TakeDamage_C"));
	}

	if (!TakeDamageWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TakeDamage feedback: Failed to load WBP_TakeDamage class."));
	}

	return TakeDamageWidgetClass != nullptr;
}

void ATeam16PlayerController::SetTakeDamageOpacity(float Opacity) const
{
	const float ClampedOpacity = FMath::Clamp(Opacity, 0.0f, 1.0f);

	if (TakeDamageWidgetInstance)
	{
		TakeDamageWidgetInstance->SetRenderOpacity(ClampedOpacity);
	}

	if (TakeDamageBloodImage)
	{
		TakeDamageBloodImage->SetRenderOpacity(ClampedOpacity);
	}
}

void ATeam16PlayerController::StopTakeDamageFeedback()
{
	SetTakeDamageOpacity(0.0f);
	bTakeDamageActive = false;
	bTakeDamageQueued = false;

	if (TakeDamageTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TakeDamageTickerHandle);
		TakeDamageTickerHandle.Reset();
	}
}

void ATeam16PlayerController::StartTakeDamageFeedback()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (TakeDamageWidgetInstance && !TakeDamageWidgetInstance->IsInViewport())
	{
		TakeDamageWidgetInstance->AddToViewport(1200);
	}

	if (TakeDamageWidgetInstance)
	{
		TakeDamageWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (TakeDamageBloodImage)
	{
		TakeDamageBloodImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	TakeDamageElapsedTime = 0.0f;
	bTakeDamageActive = true;
	bTakeDamageQueued = false;
	LastTakeDamageTriggerTime = World->GetTimeSeconds();
	SetTakeDamageOpacity(0.0f);

	if (TakeDamageTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TakeDamageTickerHandle);
		TakeDamageTickerHandle.Reset();
	}

	TakeDamageTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ATeam16PlayerController::UpdateTakeDamageFeedback)
	);
}

void ATeam16PlayerController::TriggerTakeDamageFeedback()
{
	if (!EnsureTakeDamageWidgetClass())
	{
		return;
	}

	if (!TakeDamageWidgetInstance)
	{
		TakeDamageWidgetInstance = CreateWidget<UUserWidget>(this, TakeDamageWidgetClass);
	}

	if (!TakeDamageWidgetInstance)
	{
		return;
	}

	if (!TakeDamageWidgetInstance->IsInViewport())
	{
		TakeDamageWidgetInstance->AddToViewport(1200);
	}

	if (!TakeDamageBloodImage)
	{
		TakeDamageBloodImage = Cast<UImage>(TakeDamageWidgetInstance->GetWidgetFromName(TEXT("BloodImage")));
		if (!TakeDamageBloodImage)
		{
			UE_LOG(LogTemp, Warning, TEXT("TakeDamage feedback: BloodImage widget not found. Falling back to whole-widget fade."));
		}
	}
	SetTakeDamageOpacity(0.0f);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();
	if (bTakeDamageActive && (CurrentTime - LastTakeDamageTriggerTime) < TakeDamageRetriggerInterval)
	{
		bTakeDamageQueued = true;
		return;
	}

	StartTakeDamageFeedback();
}

bool ATeam16PlayerController::UpdateTakeDamageFeedback(float DeltaTime)
{
	if (!TakeDamageWidgetInstance)
	{
		TakeDamageTickerHandle.Reset();
		bTakeDamageActive = false;
		bTakeDamageQueued = false;
		return false;
	}

	TakeDamageElapsedTime += DeltaTime;

	const float FadeIn = FMath::Max(0.01f, TakeDamageFadeInDuration);
	const float FadeOut = FMath::Max(0.05f, TakeDamageFadeOutDuration);
	const float Total = FadeIn + FadeOut;
	const float PeakOpacity = FMath::Clamp(TakeDamagePeakOpacity, 0.0f, 1.0f);

	float Opacity = 0.0f;
	if (TakeDamageElapsedTime <= FadeIn)
	{
		const float Alpha = FMath::Clamp(TakeDamageElapsedTime / FadeIn, 0.0f, 1.0f);
		Opacity = PeakOpacity * Alpha;
	}
	else if (TakeDamageElapsedTime <= Total)
	{
		const float FadeAlpha = FMath::Clamp((TakeDamageElapsedTime - FadeIn) / FadeOut, 0.0f, 1.0f);
		Opacity = PeakOpacity * (1.0f - FadeAlpha);
	}

	SetTakeDamageOpacity(Opacity);

	if (TakeDamageElapsedTime < Total)
	{
		return true;
	}

	SetTakeDamageOpacity(0.0f);
	bTakeDamageActive = false;

	UWorld* World = GetWorld();
	const bool bCanRetrigger = World && (World->GetTimeSeconds() - LastTakeDamageTriggerTime >= TakeDamageRetriggerInterval);
	if (bTakeDamageQueued && bCanRetrigger)
	{
		StartTakeDamageFeedback();
		return true;
	}

	bTakeDamageQueued = false;
	TakeDamageTickerHandle.Reset();
	return false;
}

void ATeam16PlayerController::ShowClearResult()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	HideInGameHUD();
	StopBGM();

	if (PauseMenuWidgetInstance)
	{
		PauseMenuWidgetInstance->RemoveFromParent();
		PauseMenuWidgetInstance = nullptr;
	}

	if (GameOverWidgetInstance)
	{
		GameOverWidgetInstance->RemoveFromParent();
		GameOverWidgetInstance = nullptr;
	}

	if (!ClearResultWidgetClass)
	{
		ClearResultWidgetClass = LoadClass<UClearResult>(nullptr, TEXT("/Game/UI/Menus/WBP_ClearResult.WBP_ClearResult_C"));
	}

	if (!ClearResultWidgetClass)
	{
		return;
	}

	if (!ClearResultWidgetInstance)
	{
		ClearResultWidgetInstance = CreateWidget<UClearResult>(this, ClearResultWidgetClass);
	}

	if (!ClearResultWidgetInstance)
	{
		return;
	}

	if (!ClearResultWidgetInstance->IsInViewport())
	{
		ClearResultWidgetInstance->AddToViewport(100);
	}

	ClearResultWidgetInstance->Show();
	ClearResultWidgetInstance->UpdateClearStats(
		FMath::Max(0, SurvivalElapsedSeconds),
		ZombieKillCount,
		TotalDamageTaken
	);

	UGameplayStatics::SetGamePaused(World, true);
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ClearResultWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ATeam16PlayerController::StartGameTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bBossHUDActive = false;
	SurvivalElapsedSeconds = 0;
	GameTimerRemainingSeconds = FMath::Max(0, GameTimerStartSeconds);
	UpdateHUDTime();

	World->GetTimerManager().ClearTimer(GameTimerHandle);
	World->GetTimerManager().SetTimer(GameTimerHandle, this, &ATeam16PlayerController::HandleGameTimerTick, 1.0f, true);
}

void ATeam16PlayerController::StopGameTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GameTimerHandle);
	}
}

void ATeam16PlayerController::HandleGameTimerTick()
{
	SurvivalElapsedSeconds++;

	if (GameTimerRemainingSeconds > 0)
	{
		GameTimerRemainingSeconds--;
	}

	UpdateHUDTime();
}

void ATeam16PlayerController::UpdateHUDTime()
{
	if (HUDWidgetInstance)
	{
		if (bBossHUDActive)
		{
			HUDWidgetInstance->HideTime();
			return;
		}

		if (GameTimerRemainingSeconds > 0)
		{
			HUDWidgetInstance->UpdateTime(GameTimerRemainingSeconds);
		}
		else
		{
			HUDWidgetInstance->HideTime();
		}
	}
}

void ATeam16PlayerController::OpenLevelUpUI()
{
	if (bIsLevelUpUIOpen)
	{
		CloseLevelUpUI();
		return;
	}

	ShowLevelUpUI();
}

void ATeam16PlayerController::ShowLevelUpUI()
{
	if (bIsLevelUpUIOpen)
	{
		return;
	}

	if (!LevelUpWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelUpWidgetClass is not set on Team16PlayerController."));
		return;
	}

	LevelUpWidget = CreateWidget<ULevelUpWidget>(this, LevelUpWidgetClass);
	if (!LevelUpWidget)
	{
		return;
	}

	LevelUpWidget->SetupRandomCards();
	LevelUpWidget->AddToViewport();
	bIsLevelUpUIOpen = true;

	SetPause(true);
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(LevelUpWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ATeam16PlayerController::CloseLevelUpUI()
{
	if (LevelUpWidget)
	{
		LevelUpWidget->RemoveFromParent();
		LevelUpWidget = nullptr;
	}

	bIsLevelUpUIOpen = false;
	SetPause(false);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
}

void ATeam16PlayerController::SetGameTimerStartSeconds(int32 NewStartSeconds)
{
	GameTimerStartSeconds = FMath::Max(0, NewStartSeconds);
	GameTimerRemainingSeconds = GameTimerStartSeconds;
	SurvivalElapsedSeconds = 0;
	UpdateHUDTime();
}

void ATeam16PlayerController::PlayMainMenuBGM()
{
	PlayBGMByMode(true);
}

void ATeam16PlayerController::PlayInGameBGM()
{
	PlayBGMByMode(false);
}

void ATeam16PlayerController::StopBGM()
{
	bIsStoppingBGM = true;

	if (BGMComponent)
	{
		BGMComponent->OnAudioFinished.RemoveDynamic(this, &ATeam16PlayerController::HandleBGMFinished);
		BGMComponent->Stop();
		BGMComponent = nullptr;
	}

	bIsStoppingBGM = false;
}

void ATeam16PlayerController::PlayBGMByMode(bool bUseMainMenuBGM)
{
	ResolveBGMSoundAssets();
	USoundBase* TargetSound = bUseMainMenuBGM ? MainMenuBGMSound : InGameBGMSound;
	if (!TargetSound)
	{
		return;
	}

	// 이미 같은 모드 BGM이 재생중이면 다시 재생하지 않습니다.
	if (BGMComponent && BGMComponent->IsPlaying() && bIsMainMenuBGMMode == bUseMainMenuBGM)
	{
		return;
	}

	StopBGM();
	bIsMainMenuBGMMode = bUseMainMenuBGM;
	bIsStoppingBGM = false;

	BGMComponent = UGameplayStatics::SpawnSound2D(this, TargetSound, 1.0f, 1.0f, 0.0f, nullptr, true, false);
	if (!BGMComponent)
	{
		return;
	}

	BGMComponent->bIsUISound = bUseMainMenuBGM;
	BGMComponent->SetVolumeMultiplier(GetCurrentBGMVolume());
	BGMComponent->OnAudioFinished.AddDynamic(this, &ATeam16PlayerController::HandleBGMFinished);
}

void ATeam16PlayerController::ResolveBGMSoundAssets()
{
	if (!MainMenuBGMSound)
	{
		MainMenuBGMSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/UI/MainMenu.MainMenu"));
	}

	if (!InGameBGMSound)
	{
		InGameBGMSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/UI/InGame.InGame"));
	}
}

void ATeam16PlayerController::HandleBGMFinished()
{
	if (bIsStoppingBGM || !BGMComponent)
	{
		return;
	}

	// SoundWave Loop 설정 여부와 상관없이 강제로 반복 재생합니다.
	BGMComponent->Play(0.0f);
}

void ATeam16PlayerController::SetBGMVolume(float NewVolume)
{
	MasterBGMVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	if (BGMComponent)
	{
		BGMComponent->SetVolumeMultiplier(GetCurrentBGMVolume());
	}
}

float ATeam16PlayerController::GetCurrentBGMVolume() const
{
	const float ModeVolume = bIsMainMenuBGMMode ? MainMenuBGMVolume : InGameBGMVolume;
	return FMath::Clamp(MasterBGMVolume * ModeVolume, 0.0f, 1.0f);
}
/*
void ATeam16PlayerController::CheatMaxEnhanceAllWeapons()
{
	ASP_Character* PlayerChar = Cast<ASP_Character>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!PlayerChar) return;

	TArray<EWeaponType> AllWeapons = {
		EWeaponType::Standard, EWeaponType::Old,
		EWeaponType::Supply,   EWeaponType::Spare,
		EWeaponType::Enhanced, EWeaponType::Improved,
		EWeaponType::Special
	};

	for (EWeaponType Type : AllWeapons)
	{
		if (!PlayerChar->HasWeapon(Type))
		{
			PlayerChar->OwnedWeapons.Add(FWeaponData(Type, 3));
		}
		else
		{
			for (FWeaponData& Weapon : PlayerChar->OwnedWeapons)
			{
				if (Weapon.WeaponType == Type)
				{
					Weapon.EnhanceLevel = 3;
					break;
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Cheat: All weapons max enhanced!"));
}
*/
void ATeam16PlayerController::OpenOptionUI()
{
	if (OptionWidgetClass)
	{
		// 1. 위젯 생성 및 변수에 저장
		if (!OptionWidgetInstance)
		{
			OptionWidgetInstance = CreateWidget<UOptionWidget>(this, OptionWidgetClass);
		}

		if (OptionWidgetInstance)
		{
			OptionWidgetInstance->AddToViewport();

			// 2. 초기화 및 마우스 설정
			OptionWidgetInstance->RefreshUI();
			bIsOptionUIOpen = true;

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(OptionWidgetInstance->TakeWidget());
			SetInputMode(InputMode);
			bShowMouseCursor = true;
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
}
