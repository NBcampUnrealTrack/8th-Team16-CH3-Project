// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/PauseMenu.h"

#include "Team16PlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
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
		ResumeButton->OnHovered.AddUniqueDynamic(this, &UPauseMenu::OnResumeButtonHovered);
		ResumeButton->OnUnhovered.AddUniqueDynamic(this, &UPauseMenu::OnResumeButtonUnhovered);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnRestartButtonClicked);
		RestartButton->OnHovered.AddUniqueDynamic(this, &UPauseMenu::OnRestartButtonHovered);
		RestartButton->OnUnhovered.AddUniqueDynamic(this, &UPauseMenu::OnRestartButtonUnhovered);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnMainMenuButtonClicked);
		MainMenuButton->OnHovered.AddUniqueDynamic(this, &UPauseMenu::OnMainMenuButtonHovered);
		MainMenuButton->OnUnhovered.AddUniqueDynamic(this, &UPauseMenu::OnMainMenuButtonUnhovered);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnQuitButtonClicked);
		QuitButton->OnHovered.AddUniqueDynamic(this, &UPauseMenu::OnQuitButtonHovered);
		QuitButton->OnUnhovered.AddUniqueDynamic(this, &UPauseMenu::OnQuitButtonUnhovered);
	}

	SetButtonImageHovered(ResumeImage, false);
	SetButtonImageHovered(RestartImage, false);
	SetButtonImageHovered(MainMenuImage, false);
	SetButtonImageHovered(QuitImage, false);
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

void UPauseMenu::OnResumeButtonHovered()
{
	SetButtonImageHovered(ResumeImage, true);
}

void UPauseMenu::OnResumeButtonUnhovered()
{
	SetButtonImageHovered(ResumeImage, false);
}

void UPauseMenu::OnRestartButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);

	UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), true, TEXT("SkipMainMenu"));
}

void UPauseMenu::OnRestartButtonHovered()
{
	SetButtonImageHovered(RestartImage, true);
}

void UPauseMenu::OnRestartButtonUnhovered()
{
	SetButtonImageHovered(RestartImage, false);
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

	UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), true, TEXT("ShowMainMenu"));
}

void UPauseMenu::OnMainMenuButtonHovered()
{
	SetButtonImageHovered(MainMenuImage, true);
}

void UPauseMenu::OnMainMenuButtonUnhovered()
{
	SetButtonImageHovered(MainMenuImage, false);
}

void UPauseMenu::OnQuitButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;

	UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
}

void UPauseMenu::OnQuitButtonHovered()
{
	SetButtonImageHovered(QuitImage, true);
}

void UPauseMenu::OnQuitButtonUnhovered()
{
	SetButtonImageHovered(QuitImage, false);
}

void UPauseMenu::SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const
{
	if (!ButtonImage)
	{
		return;
	}

	ButtonImage->SetColorAndOpacity(bIsHovered ? ButtonImageHoveredTint : ButtonImageNormalTint);
}
