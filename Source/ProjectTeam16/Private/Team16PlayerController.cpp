// Fill out your copyright notice in the Description page of Project Settings.

#include "Team16PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/UI/GameOver.h"
#include "ProjectTeam16/UI/InGameHUD.h"
#include "ProjectTeam16/UI/LevelUpWidget.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"

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
	InputComponent->BindAction("CheatMaxEnhance", IE_Pressed,
		this, &ATeam16PlayerController::CheatMaxEnhanceAllWeapons);
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

	
	if (ASP_Character* PlayerCharacter = Cast<ASP_Character>(GetPawn()))
	{
		PlayerCharacter->SyncHUDValues();
	}
	HUDWidgetInstance->UpdateZombieKillCount(ZombieKillCount);

	StartGameTimer();
}

void ATeam16PlayerController::HideInGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->Hide();
	}

	StopGameTimer();
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
	if (HUDWidgetInstance)
	{
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
	}
}

void ATeam16PlayerController::ShowGameOver()
{
	UWorld* World = GetWorld();
	if (!World || !GameOverWidgetClass)
	{
		return;
	}

	HideInGameHUD();

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

	UGameplayStatics::SetGamePaused(World, true);

	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GameOverWidgetInstance->TakeWidget());
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
	if (GameTimerRemainingSeconds > 0)
	{
		GameTimerRemainingSeconds--;
		UpdateHUDTime();
	}

	if (GameTimerRemainingSeconds <= 0)
	{
		StopGameTimer();
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->HideTime();
			HUDWidgetInstance->ShowBossHealth(TEXT("Titan Zombie"), 1000.0f, 1000.0f);
		}
	}
}

void ATeam16PlayerController::UpdateHUDTime()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->UpdateTime(GameTimerRemainingSeconds);
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
	UpdateHUDTime();
}

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

