// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/MainMenu.h"
#include "Team16PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::OnStartButtonClicked);
		StartButton->OnHovered.AddUniqueDynamic(this, &UMainMenu::OnStartButtonHovered);
		StartButton->OnUnhovered.AddUniqueDynamic(this, &UMainMenu::OnStartButtonUnhovered);
	}

	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::OnOptionsButtonClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::OnQuitButtonClicked);
		QuitButton->OnHovered.AddUniqueDynamic(this, &UMainMenu::OnQuitButtonHovered);
		QuitButton->OnUnhovered.AddUniqueDynamic(this, &UMainMenu::OnQuitButtonUnhovered);
	}

	SetButtonImageHovered(StartButtonImage, false);
	SetButtonImageHovered(QuitButtonImage, false);
}

void UMainMenu::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UMainMenu::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UMainMenu::OnStartButtonClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	RemoveFromParent();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(PC))
		{
			Team16PlayerController->ShowInGameHUD();
		}
	}
}

void UMainMenu::OnStartButtonHovered()
{
	SetButtonImageHovered(StartButtonImage, true);
}

void UMainMenu::OnStartButtonUnhovered()
{
	SetButtonImageHovered(StartButtonImage, false);
}

void UMainMenu::OnOptionsButtonClicked()
{
}

void UMainMenu::OnQuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

void UMainMenu::OnQuitButtonHovered()
{
	SetButtonImageHovered(QuitButtonImage, true);
}

void UMainMenu::OnQuitButtonUnhovered()
{
	SetButtonImageHovered(QuitButtonImage, false);
}

void UMainMenu::SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const
{
	if (!ButtonImage)
	{
		return;
	}

	ButtonImage->SetColorAndOpacity(bIsHovered ? ButtonImageHoveredTint : ButtonImageNormalTint);
}
