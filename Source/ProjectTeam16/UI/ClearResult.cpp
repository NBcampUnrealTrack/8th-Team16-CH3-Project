// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/ClearResult.h"

#include "Team16PlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UClearResult::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddUniqueDynamic(this, &UClearResult::OnContinueButtonClicked);
		ContinueButton->OnHovered.AddUniqueDynamic(this, &UClearResult::OnContinueButtonHovered);
		ContinueButton->OnUnhovered.AddUniqueDynamic(this, &UClearResult::OnContinueButtonUnhovered);
	}

	if (RetryButton)
	{
		RetryButton->OnClicked.AddUniqueDynamic(this, &UClearResult::OnRetryButtonClicked);
		RetryButton->OnHovered.AddUniqueDynamic(this, &UClearResult::OnRetryButtonHovered);
		RetryButton->OnUnhovered.AddUniqueDynamic(this, &UClearResult::OnRetryButtonUnhovered);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UClearResult::OnMainMenuButtonClicked);
		MainMenuButton->OnHovered.AddUniqueDynamic(this, &UClearResult::OnMainMenuButtonHovered);
		MainMenuButton->OnUnhovered.AddUniqueDynamic(this, &UClearResult::OnMainMenuButtonUnhovered);
	}

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.AddUniqueDynamic(this, &UClearResult::OnQuitGameButtonClicked);
		QuitGameButton->OnHovered.AddUniqueDynamic(this, &UClearResult::OnQuitGameButtonHovered);
		QuitGameButton->OnUnhovered.AddUniqueDynamic(this, &UClearResult::OnQuitGameButtonUnhovered);
	}

	SetButtonHovered(ContinueButton, nullptr, false);
	SetButtonHovered(RetryButton, RetryButtonImage, false);
	SetButtonHovered(MainMenuButton, MainMenuButtonImage, false);
	SetButtonHovered(QuitGameButton, QuitButtonImage, false);
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
	if (KillCountValueText)
	{
		KillCountValueText->SetText(FText::AsNumber(KillCount));
	}

	if (ScoreText)
	{
		ScoreText->SetText(FText::AsNumber(KillCount));
	}
}

void UClearResult::UpdateClearTime(int32 ClearSeconds)
{
	const int32 ClampedSeconds = FMath::Max(0, ClearSeconds);
	const int32 Minutes = ClampedSeconds / 60;
	const int32 Seconds = ClampedSeconds % 60;
	const FText TimeText = FText::FromString(FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds));

	if (ClearTimeText)
	{
		ClearTimeText->SetText(TimeText);
	}

	if (SurvivalTimeValueText)
	{
		SurvivalTimeValueText->SetText(TimeText);
	}

	if (ClearProgressBar)
	{
		ClearProgressBar->SetPercent(1.0f);
	}
}

void UClearResult::UpdateClearStats(int32 ClearSeconds, int32 KillCount, float DamageTaken)
{
	UpdateClearTime(ClearSeconds);
	UpdateScore(KillCount);

	const int32 DamageTakenValue = FMath::Max(0, FMath::RoundToInt(DamageTaken));
	if (DamageTakenValueText)
	{
		DamageTakenValueText->SetText(FText::AsNumber(DamageTakenValue));
	}

	if (RankValueText)
	{
		RankValueText->SetText(FText::FromString(GetRankForDamage(DamageTaken)));
	}
}

void UClearResult::OnContinueButtonClicked()
{
}

void UClearResult::OnContinueButtonHovered()
{
	SetButtonHovered(ContinueButton, nullptr, true);
}

void UClearResult::OnContinueButtonUnhovered()
{
	SetButtonHovered(ContinueButton, nullptr, false);
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

void UClearResult::OnRetryButtonHovered()
{
	SetButtonHovered(RetryButton, RetryButtonImage, true);
}

void UClearResult::OnRetryButtonUnhovered()
{
	SetButtonHovered(RetryButton, RetryButtonImage, false);
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

void UClearResult::OnMainMenuButtonHovered()
{
	SetButtonHovered(MainMenuButton, MainMenuButtonImage, true);
}

void UClearResult::OnMainMenuButtonUnhovered()
{
	SetButtonHovered(MainMenuButton, MainMenuButtonImage, false);
}

void UClearResult::OnQuitGameButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;

	UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
}

void UClearResult::OnQuitGameButtonHovered()
{
	SetButtonHovered(QuitGameButton, QuitButtonImage, true);
}

void UClearResult::OnQuitGameButtonUnhovered()
{
	SetButtonHovered(QuitGameButton, QuitButtonImage, false);
}

FString UClearResult::GetRankForDamage(float DamageTaken) const
{
	if (DamageTaken <= 0.0f)
	{
		return TEXT("S");
	}

	if (DamageTaken <= 20.0f)
	{
		return TEXT("A");
	}

	if (DamageTaken <= 40.0f)
	{
		return TEXT("A");
	}

	if (DamageTaken <= 80.0f)
	{
		return TEXT("B");
	}

	if (DamageTaken <= 120.0f)
	{
		return TEXT("C");
	}

	return TEXT("D");
}

void UClearResult::SetButtonHovered(UButton* TargetButton, UImage* TargetImage, bool bIsHovered) const
{
	const FLinearColor TargetTint = bIsHovered ? ButtonHoveredTint : ButtonNormalTint;

	if (TargetButton)
	{
		TargetButton->SetColorAndOpacity(TargetTint);
	}

	if (TargetImage)
	{
		TargetImage->SetColorAndOpacity(TargetTint);
	}
}
