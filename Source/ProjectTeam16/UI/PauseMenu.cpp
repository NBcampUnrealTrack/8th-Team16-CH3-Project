// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/PauseMenu.h"

#include "Components/Button.h"

void UPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

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
}

void UPauseMenu::OnRestartButtonClicked()
{
}

void UPauseMenu::OnMainMenuButtonClicked()
{
}

void UPauseMenu::OnQuitButtonClicked()
{
}
