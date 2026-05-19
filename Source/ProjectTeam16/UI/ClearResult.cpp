// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/ClearResult.h"

#include "Team16PlayerController.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

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

void UClearResult::UpdateScore(int32 KillCount)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("죽인 좀비 수: %d"), KillCount)));
	}
}

void UClearResult::UpdateClearTime(int32 ClearSeconds)
{
	const int32 ClampedSeconds = FMath::Max(0, ClearSeconds);
	const int32 Minutes = ClampedSeconds / 60;
	const int32 Seconds = ClampedSeconds % 60;

	if (ClearTimeText)
	{
		ClearTimeText->SetText(FText::FromString(FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds)));
	}

	if (ClearProgressBar)
	{
		ClearProgressBar->SetPercent(1.0f);
	}
}

void UClearResult::OnContinueButtonClicked()
{
}

void UClearResult::OnRetryButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(GetOwningPlayer()))
	{
		PlayerController->FadeToLevel(FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), TEXT("SkipMainMenu"));
		return;
	}

	UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), true, TEXT("SkipMainMenu"));
}

void UClearResult::OnMainMenuButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ATeam16PlayerController* PlayerController = Cast<ATeam16PlayerController>(GetOwningPlayer()))
	{
		PlayerController->FadeToLevel(FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), TEXT("ShowMainMenu"));
		return;
	}

	UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Modern_Gas_Station/Maps/MainLevel")), true, TEXT("ShowMainMenu"));
}
