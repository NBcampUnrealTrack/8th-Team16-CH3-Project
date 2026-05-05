// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/ClearResult.h"

#include "Components/Button.h"

void UClearResult::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddUniqueDynamic(this, &UClearResult::OnContinueButtonClicked);
	}

	if (RetryButton)
	{
		RetryButton->OnClicked.AddUniqueDynamic(this, &UClearResult::OnRetryButtonClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UClearResult::OnMainMenuButtonClicked);
	}
}

void UClearResult::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UClearResult::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UClearResult::OnContinueButtonClicked()
{
}

void UClearResult::OnRetryButtonClicked()
{
}

void UClearResult::OnMainMenuButtonClicked()
{
}
