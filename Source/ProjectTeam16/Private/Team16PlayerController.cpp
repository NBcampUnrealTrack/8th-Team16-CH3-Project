// Fill out your copyright notice in the Description page of Project Settings.

#include "Team16PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/UI/InGameHUD.h"

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

	// HUD가 처음 표시될 때 플레이어가 가진 실제 체력/경험치/레벨 값을 반영합니다.
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

void ATeam16PlayerController::RegisterZombieKill(int32 ExpReward)
{
	ZombieKillCount++;
	UpdateHUDZombieKillCount(ZombieKillCount);

	// 좀비 처치 경험치는 플레이어 캐릭터가 보관하고 HUD까지 갱신합니다.
	if (ASP_Character* PlayerCharacter = Cast<ASP_Character>(GetPawn()))
	{
		PlayerCharacter->AddExperience(ExpReward);
	}
}

void ATeam16PlayerController::StartGameTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// HUD가 나타나는 순간부터 10분 카운트다운을 시작하고 TimeText에 표시합니다.
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
