// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/InGameHUD.h"

#include "Components/Button.h"

void UInGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (PauseButton)
	{
		PauseButton->OnClicked.AddUniqueDynamic(this, &UInGameHUD::OnPauseButtonClicked);
	}
}

void UInGameHUD::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UInGameHUD::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInGameHUD::OnPauseButtonClicked()
{
}
