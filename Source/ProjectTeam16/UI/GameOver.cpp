// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/GameOver.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInterface.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "UObject/ConstructorHelpers.h"

UGameOver::UGameOver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMediaPlayer> DefaultMediaPlayer(TEXT("/Game/UI/Videos/MP_GameOver.MP_GameOver"));
	if (DefaultMediaPlayer.Succeeded())
	{
		GameOverMediaPlayer = DefaultMediaPlayer.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMediaSource> DefaultMediaSource(TEXT("/Game/UI/Videos/GameOver.GameOver"));
	if (DefaultMediaSource.Succeeded())
	{
		GameOverMediaSource = DefaultMediaSource.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultVideoMaterial(TEXT("/Game/UI/Videos/M_GameOver_Video_UI.M_GameOver_Video_UI"));
	if (DefaultVideoMaterial.Succeeded())
	{
		GameOverVideoMaterial = DefaultVideoMaterial.Object;
	}
}

void UGameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if (RetryButton)
	{
		RetryButton->OnClicked.AddUniqueDynamic(this, &UGameOver::OnRetryButtonClicked);
		RetryButton->OnHovered.AddUniqueDynamic(this, &UGameOver::OnRetryButtonHovered);
		RetryButton->OnUnhovered.AddUniqueDynamic(this, &UGameOver::OnRetryButtonUnhovered);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UGameOver::OnMainMenuButtonClicked);
		MainMenuButton->OnHovered.AddUniqueDynamic(this, &UGameOver::OnMainMenuButtonHovered);
		MainMenuButton->OnUnhovered.AddUniqueDynamic(this, &UGameOver::OnMainMenuButtonUnhovered);
	}

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.AddUniqueDynamic(this, &UGameOver::OnQuitGameButtonClicked);
		QuitGameButton->OnHovered.AddUniqueDynamic(this, &UGameOver::OnQuitGameButtonHovered);
		QuitGameButton->OnUnhovered.AddUniqueDynamic(this, &UGameOver::OnQuitGameButtonUnhovered);
	}

	SetButtonImageHovered(RetryButtonImage, false);
	SetButtonImageHovered(MainMenuButtonImage, false);
	SetButtonImageHovered(QuitGameButtonImage, false);
	StartGameOverBackgroundVideo();
}

void UGameOver::NativeDestruct()
{
	StopGameOverBackgroundVideo();

	Super::NativeDestruct();
}

void UGameOver::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	StartGameOverBackgroundVideo();
}

void UGameOver::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
	StopGameOverBackgroundVideo();
}

void UGameOver::UpdateScore(int32 KillCount)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::AsNumber(KillCount));
	}
}

void UGameOver::UpdateSurvivalTime(int32 SurvivalSeconds)
{
	const int32 ClampedSeconds = FMath::Max(0, SurvivalSeconds);
	const int32 Minutes = ClampedSeconds / 60;
	const int32 Seconds = ClampedSeconds % 60;

	if (SurvivalTimeText)
	{
		SurvivalTimeText->SetText(FText::FromString(FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds)));
	}
}

void UGameOver::OnRetryButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);

	UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), true, TEXT("SkipMainMenu"));
}

void UGameOver::OnRetryButtonHovered()
{
	SetButtonImageHovered(RetryButtonImage, true);
}

void UGameOver::OnRetryButtonUnhovered()
{
	SetButtonImageHovered(RetryButtonImage, false);
}

void UGameOver::OnMainMenuButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);
	UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), true, TEXT("ShowMainMenu"));
}

void UGameOver::OnMainMenuButtonHovered()
{
	SetButtonImageHovered(MainMenuButtonImage, true);
}

void UGameOver::OnMainMenuButtonUnhovered()
{
	SetButtonImageHovered(MainMenuButtonImage, false);
}

void UGameOver::OnQuitGameButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;

	UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
}

void UGameOver::OnQuitGameButtonHovered()
{
	SetButtonImageHovered(QuitGameButtonImage, true);
}

void UGameOver::OnQuitGameButtonUnhovered()
{
	SetButtonImageHovered(QuitGameButtonImage, false);
}

void UGameOver::HandleGameOverVideoOpened(FString OpenedUrl)
{
	if (GameOverMediaPlayer)
	{
		GameOverMediaPlayer->Play();
	}
}

void UGameOver::HandleGameOverVideoEndReached()
{
	if (GameOverMediaPlayer)
	{
		GameOverMediaPlayer->Rewind();
		GameOverMediaPlayer->Play();
	}
}

void UGameOver::StartGameOverBackgroundVideo()
{
	if (GameOver && GameOverVideoMaterial)
	{
		GameOver->SetBrushFromMaterial(GameOverVideoMaterial);
	}

	if (!GameOverMediaPlayer || !GameOverMediaSource)
	{
		return;
	}

	GameOverMediaPlayer->OnMediaOpened.AddUniqueDynamic(this, &UGameOver::HandleGameOverVideoOpened);
	GameOverMediaPlayer->OnEndReached.AddUniqueDynamic(this, &UGameOver::HandleGameOverVideoEndReached);
	GameOverMediaPlayer->SetLooping(true);

	if (!GameOverMediaPlayer->IsPlaying())
	{
		GameOverMediaPlayer->OpenSource(GameOverMediaSource);
	}
}

void UGameOver::StopGameOverBackgroundVideo()
{
	if (!GameOverMediaPlayer)
	{
		return;
	}

	GameOverMediaPlayer->OnMediaOpened.RemoveDynamic(this, &UGameOver::HandleGameOverVideoOpened);
	GameOverMediaPlayer->OnEndReached.RemoveDynamic(this, &UGameOver::HandleGameOverVideoEndReached);
	GameOverMediaPlayer->Close();
}

void UGameOver::SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const
{
	if (!ButtonImage)
	{
		return;
	}

	ButtonImage->SetColorAndOpacity(bIsHovered ? ButtonImageHoveredTint : ButtonImageNormalTint);
}
