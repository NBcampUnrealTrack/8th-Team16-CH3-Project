// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/GameOver.h"

#include "Components/Button.h"

void UGameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if (RetryButton)
	{
		RetryButton->OnClicked.AddUniqueDynamic(this, &UGameOver::OnRetryButtonClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UGameOver::OnMainMenuButtonClicked);
	}
}

void UGameOver::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UGameOver::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGameOver::OnRetryButtonClicked()
{
}

void UGameOver::OnMainMenuButtonClicked()
{
}
