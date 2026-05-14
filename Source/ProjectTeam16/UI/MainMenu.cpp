// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/MainMenu.h"
#include "Team16PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Materials/MaterialInterface.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "UObject/ConstructorHelpers.h"

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMediaPlayer> DefaultMediaPlayer(TEXT("/Game/UI/Videos/MP_MainMenu.MP_MainMenu"));
	if (DefaultMediaPlayer.Succeeded())
	{
		MainMenuMediaPlayer = DefaultMediaPlayer.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMediaSource> DefaultMediaSource(TEXT("/Game/UI/Videos/MainMenu.MainMenu"));
	if (DefaultMediaSource.Succeeded())
	{
		MainMenuMediaSource = DefaultMediaSource.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultVideoMaterial(TEXT("/Game/UI/Videos/M_MainMenuVideo_UI.M_MainMenuVideo_UI"));
	if (DefaultVideoMaterial.Succeeded())
	{
		MainMenuVideoMaterial = DefaultVideoMaterial.Object;
	}
}

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
	StartMainMenuBackgroundVideo();
}

void UMainMenu::NativeDestruct()
{
	StopMainMenuBackgroundVideo();

	Super::NativeDestruct();
}

void UMainMenu::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	StartMainMenuBackgroundVideo();
}

void UMainMenu::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
	StopMainMenuBackgroundVideo();
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

void UMainMenu::HandleMainMenuVideoOpened(FString OpenedUrl)
{
	if (MainMenuMediaPlayer)
	{
		MainMenuMediaPlayer->Play();
	}
}

void UMainMenu::HandleMainMenuVideoEndReached()
{
	if (MainMenuMediaPlayer)
	{
		MainMenuMediaPlayer->Rewind();
		MainMenuMediaPlayer->Play();
	}
}

void UMainMenu::SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const
{
	if (!ButtonImage)
	{
		return;
	}

	ButtonImage->SetColorAndOpacity(bIsHovered ? ButtonImageHoveredTint : ButtonImageNormalTint);
}

void UMainMenu::StartMainMenuBackgroundVideo()
{
	if (BackgroundVideoImage && MainMenuVideoMaterial)
	{
		BackgroundVideoImage->SetBrushFromMaterial(MainMenuVideoMaterial);
	}

	if (!MainMenuMediaPlayer || !MainMenuMediaSource)
	{
		return;
	}

	MainMenuMediaPlayer->OnMediaOpened.AddUniqueDynamic(this, &UMainMenu::HandleMainMenuVideoOpened);
	MainMenuMediaPlayer->OnEndReached.AddUniqueDynamic(this, &UMainMenu::HandleMainMenuVideoEndReached);
	MainMenuMediaPlayer->SetLooping(true);

	if (!MainMenuMediaPlayer->IsPlaying())
	{
		MainMenuMediaPlayer->OpenSource(MainMenuMediaSource);
	}
}

void UMainMenu::StopMainMenuBackgroundVideo()
{
	if (!MainMenuMediaPlayer)
	{
		return;
	}

	MainMenuMediaPlayer->OnMediaOpened.RemoveDynamic(this, &UMainMenu::HandleMainMenuVideoOpened);
	MainMenuMediaPlayer->OnEndReached.RemoveDynamic(this, &UMainMenu::HandleMainMenuVideoEndReached);
	MainMenuMediaPlayer->Close();
}
