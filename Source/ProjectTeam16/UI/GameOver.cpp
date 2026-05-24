// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/GameOver.h"

#include "Team16PlayerController.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	float CalculateAlphaInRange(float Time, float Start, float Duration)
	{
		if (Duration <= KINDA_SMALL_NUMBER)
		{
			return Time >= Start ? 1.0f : 0.0f;
		}

		return FMath::Clamp((Time - Start) / Duration, 0.0f, 1.0f);
	}
}

void UGameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UGameOver::OnMainMenuButtonClicked);
	}

	ResolveOptionalWidgets();
	UpdateScore(CachedKillCount);
	UpdateSurvivalTime(CachedSurvivalSeconds);
	InitializeRevealState();
}

void UGameOver::NativeDestruct()
{
	StopRevealSequence();
	Super::NativeDestruct();
}

void UGameOver::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	InitializeRevealState();
	StartRevealSequence();
}

void UGameOver::Hide()
{
	StopRevealSequence();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGameOver::UpdateScore(int32 KillCount)
{
	CachedKillCount = FMath::Max(0, KillCount);

	if (Killcount)
	{
		Killcount->SetText(FText::AsNumber(CachedKillCount));
	}

	if (Killcount2)
	{
		Killcount2->SetText(FText::FromString(TEXT("처치 수:")));
	}
}

void UGameOver::UpdateSurvivalTime(int32 SurvivalSeconds)
{
	CachedSurvivalSeconds = FMath::Max(0, SurvivalSeconds);
	const int32 Minutes = CachedSurvivalSeconds / 60;
	const int32 Seconds = CachedSurvivalSeconds % 60;
	const FString TimeValue = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);

	if (Time)
	{
		Time->SetText(FText::FromString(TimeValue));
	}

	if (Time2)
	{
		Time2->SetText(FText::FromString(TEXT("생존 시간:")));
	}
}

void UGameOver::OnMainMenuButtonClicked()
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

void UGameOver::InitializeRevealState()
{
	StopRevealSequence();

	if (Border_0)
	{
		Border_0->SetBrushColor(RedOverlayColor);
	}

	ApplyWidgetReveal(Border_0, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(GameOverImage, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(GameOverImage2, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Killcount, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Killcount2, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(KillcountImage, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Time, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Time2, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(TimeImage, 0.0f, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(MainMenuButton, 0.0f, ESlateVisibility::Visible);

	if (MainMenuButton)
	{
		MainMenuButton->SetIsEnabled(false);
	}
}

void UGameOver::StartRevealSequence()
{
	RevealElapsedTime = 0.0f;

	if (RevealTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RevealTickerHandle);
		RevealTickerHandle.Reset();
	}

	RevealTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UGameOver::UpdateRevealSequence));
}

void UGameOver::StopRevealSequence()
{
	if (RevealTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RevealTickerHandle);
		RevealTickerHandle.Reset();
	}
}

bool UGameOver::UpdateRevealSequence(float DeltaTime)
{
	RevealElapsedTime += DeltaTime;

	const float Stage1Start = 0.0f;
	const float Stage2Start = Stage1Start + RedFadeDuration + StageGapDuration;
	const float Stage3Start = Stage2Start + GameOverImagesFadeDuration + StageGapDuration;
	const float Stage4Start = Stage3Start + StatsFadeDuration + StageGapDuration;
	const float EndTime = Stage4Start + MainMenuButtonFadeDuration;

	const float RedAlpha = CalculateAlphaInRange(RevealElapsedTime, Stage1Start, RedFadeDuration) * RedOverlayTargetOpacity;
	const float ImagesAlpha = CalculateAlphaInRange(RevealElapsedTime, Stage2Start, GameOverImagesFadeDuration);
	const float StatsAlpha = CalculateAlphaInRange(RevealElapsedTime, Stage3Start, StatsFadeDuration);
	const float ButtonAlpha = CalculateAlphaInRange(RevealElapsedTime, Stage4Start, MainMenuButtonFadeDuration);

	ApplyWidgetReveal(Border_0, RedAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(GameOverImage, ImagesAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(GameOverImage2, ImagesAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Killcount, StatsAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Killcount2, StatsAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(KillcountImage, StatsAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Time, StatsAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(Time2, StatsAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(TimeImage, StatsAlpha, ESlateVisibility::HitTestInvisible);
	ApplyWidgetReveal(MainMenuButton, ButtonAlpha, ESlateVisibility::Visible);

	if (MainMenuButton)
	{
		MainMenuButton->SetIsEnabled(ButtonAlpha >= 1.0f);
	}

	return RevealElapsedTime < EndTime;
}

void UGameOver::ApplyWidgetReveal(UWidget* TargetWidget, float Alpha, ESlateVisibility VisibleState) const
{
	if (!TargetWidget)
	{
		return;
	}

	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	TargetWidget->SetVisibility(ClampedAlpha > 0.0f ? VisibleState : ESlateVisibility::Hidden);
	TargetWidget->SetRenderOpacity(ClampedAlpha);
}

void UGameOver::ResolveOptionalWidgets()
{
	if (!Border_0)
	{
		Border_0 = Cast<UBorder>(GetWidgetFromName(TEXT("Border_0")));
	}

	if (!GameOverImage)
	{
		GameOverImage = Cast<UImage>(GetWidgetFromName(TEXT("GameOverImage")));
	}

	if (!GameOverImage2)
	{
		GameOverImage2 = Cast<UImage>(GetWidgetFromName(TEXT("GameOverImage2")));
		if (!GameOverImage2)
		{
			GameOverImage2 = Cast<UImage>(GetWidgetFromName(TEXT("Image")));
		}
	}

	if (!KillcountImage)
	{
		KillcountImage = Cast<UImage>(GetWidgetFromName(TEXT("KillcountImage")));
	}

	if (!TimeImage)
	{
		TimeImage = Cast<UImage>(GetWidgetFromName(TEXT("TimeImage")));
	}

	if (!Killcount)
	{
		Killcount = Cast<UTextBlock>(GetWidgetFromName(TEXT("Killcount")));
		if (!Killcount)
		{
			Killcount = Cast<UTextBlock>(GetWidgetFromName(TEXT("ScoreText")));
		}
	}

	if (!Killcount2)
	{
		Killcount2 = Cast<UTextBlock>(GetWidgetFromName(TEXT("Killcount2")));
	}

	if (!Time)
	{
		Time = Cast<UTextBlock>(GetWidgetFromName(TEXT("Time")));
		if (!Time)
		{
			Time = Cast<UTextBlock>(GetWidgetFromName(TEXT("SurvivalTimeText")));
		}
	}

	if (!Time2)
	{
		Time2 = Cast<UTextBlock>(GetWidgetFromName(TEXT("Time2")));
	}
}
