// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/PauseMenu.h"

#include "Team16PlayerController.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnResumeButtonClicked);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnRestartButtonClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnMainMenuButtonClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnQuitButtonClicked);
	}
}

FReply UPauseMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(GetOwningPlayer()))
		{
			PlayerController->TogglePauseMenu();
		}
		else
		{
			OnResumeButtonClicked();
		}

		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenu::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UPauseMenu::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UPauseMenu::OnResumeButtonClicked()
{
	if (ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(GetOwningPlayer()))
	{
		PlayerController->TogglePauseMenu();
		return;
	}

	RemoveFromParent();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
}

void UPauseMenu::OnRestartButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName), true, TEXT("SkipMainMenu"));
	}
}

void UPauseMenu::OnMainMenuButtonClicked()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);

	UGameplayStatics::OpenLevel(World, FName(TEXT("MainLevel")), true, TEXT("ShowMainMenu"));
}

void UPauseMenu::OnQuitButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;

	UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
}
