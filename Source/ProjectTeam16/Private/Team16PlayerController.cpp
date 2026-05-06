// Fill out your copyright notice in the Description page of Project Settings.

#include "Team16PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"

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
