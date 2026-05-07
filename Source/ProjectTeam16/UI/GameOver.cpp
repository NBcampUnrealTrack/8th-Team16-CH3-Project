// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/GameOver.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.AddUniqueDynamic(this, &UGameOver::OnQuitGameButtonClicked);
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

void UGameOver::UpdateScore(int32 KillCount)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("죽인 좀비 수: %d"), KillCount)));
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

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName), true, TEXT("SkipMainMenu"));
	}
}

void UGameOver::OnMainMenuButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);
	UGameplayStatics::OpenLevel(World, FName(TEXT("MainLevel")), true, TEXT("ShowMainMenu"));
}

void UGameOver::OnQuitGameButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;

	UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
}
