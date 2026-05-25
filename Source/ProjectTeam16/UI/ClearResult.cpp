// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/ClearResult.h"

#include "Team16PlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

void UClearResult::NativeConstruct()
{
	Super::NativeConstruct();

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

	SetButtonHovered(RetryButton, RetryButtonImage, false);
	SetButtonHovered(MainMenuButton, MainMenuButtonImage, false);
	SetButtonHovered(QuitGameButton, QuitButtonImage, false);

	ResolveOptionalWidgets();
	InitializeRevealState();
}

void UClearResult::NativeDestruct()
{
	StopRevealSequence();
	Super::NativeDestruct();
}

void UClearResult::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	ResolveOptionalWidgets();
	InitializeRevealState();
	StartRevealSequence();
}

void UClearResult::Hide()
{
	StopRevealSequence();
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

void UClearResult::ResolveOptionalWidgets()
{
	if (!Image_0)
	{
		Image_0 = Cast<UImage>(GetWidgetFromName(TEXT("Image_0")));
	}

	if (!BackgroundImage)
	{
		BackgroundImage = Cast<UImage>(GetWidgetFromName(TEXT("Image")));
		if (!BackgroundImage)
		{
			BackgroundImage = Cast<UImage>(GetWidgetFromName(TEXT("BackgroundVideoImage")));
		}
		if (!BackgroundImage)
		{
			BackgroundImage = Cast<UImage>(GetWidgetFromName(TEXT("BackgroundVideoImage_1")));
		}
		if (!BackgroundImage)
		{
			BackgroundImage = Cast<UImage>(GetWidgetFromName(TEXT("BackgroundVideoImage1")));
		}
	}

	if (!GameOverImage)
	{
		GameOverImage = Cast<UImage>(GetWidgetFromName(TEXT("GameOverImage")));
	}

	if (!GameOverImage2)
	{
		GameOverImage2 = Cast<UImage>(GetWidgetFromName(TEXT("GameOverImage2")));
	}

	if (!KillCountValueText2)
	{
		KillCountValueText2 = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillCountValueText2")));
	}

	if (!SurvivalTimeValueText2)
	{
		SurvivalTimeValueText2 = Cast<UTextBlock>(GetWidgetFromName(TEXT("SurvivalTimeValueText2")));
	}

	if (!DamageTakenValueText2)
	{
		DamageTakenValueText2 = Cast<UTextBlock>(GetWidgetFromName(TEXT("DamageTakenValueText2")));
	}

	if (!RankValueText2)
	{
		RankValueText2 = Cast<UTextBlock>(GetWidgetFromName(TEXT("RankValueText2")));
	}
}

void UClearResult::InitializeRevealState()
{
	StopRevealSequence();
	RevealItems.Reset();

	auto HideWidget = [this](UWidget* TargetWidget, ESlateVisibility VisibleState)
	{
		ApplyWidgetReveal(TargetWidget, 0.0f, VisibleState);
	};

	if (Image_0)
	{
		Image_0->SetVisibility(ESlateVisibility::HitTestInvisible);
		Image_0->SetRenderOpacity(1.0f);
	}

	// ClearResult의 기본 배경은 항상 유지합니다. (연출 대상에서 제외)
	if (BackgroundImage)
	{
		BackgroundImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		BackgroundImage->SetRenderOpacity(1.0f);
	}

	HideWidget(GameOverImage, ESlateVisibility::HitTestInvisible);
	HideWidget(GameOverImage2, ESlateVisibility::HitTestInvisible);
	HideWidget(KillCountValueText2, ESlateVisibility::HitTestInvisible);
	HideWidget(KillCountValueText, ESlateVisibility::HitTestInvisible);
	HideWidget(SurvivalTimeValueText2, ESlateVisibility::HitTestInvisible);
	HideWidget(SurvivalTimeValueText, ESlateVisibility::HitTestInvisible);
	HideWidget(DamageTakenValueText2, ESlateVisibility::HitTestInvisible);
	HideWidget(DamageTakenValueText, ESlateVisibility::HitTestInvisible);
	HideWidget(RankValueText2, ESlateVisibility::HitTestInvisible);
	HideWidget(RankValueText, ESlateVisibility::HitTestInvisible);
	HideWidget(MainMenuButton, ESlateVisibility::Visible);

	if (MainMenuButton)
	{
		MainMenuButton->SetIsEnabled(false);
	}

	if (GameOverImage)
	{
		GameOverImageBaseTransform = GameOverImage->GetRenderTransform();
	}

	if (GameOverImage2)
	{
		GameOverImage2BaseTransform = GameOverImage2->GetRenderTransform();
	}

	const float SequenceStart = OverlayFadeOutDuration + TitlePopDuration + TitleShakeDuration + TitleToStatsDelay;
	float CurrentStart = SequenceStart;
	auto QueueItem = [this, &CurrentStart](UWidget* TargetWidget, bool bEnableOnFinish = false)
	{
		if (!TargetWidget)
		{
			CurrentStart += StatItemInterval;
			return;
		}

		FRevealItem& NewItem = RevealItems.Emplace_GetRef();
		NewItem.Widget = TargetWidget;
		NewItem.BaseTransform = TargetWidget->GetRenderTransform();
		NewItem.StartTime = CurrentStart;
		NewItem.bEnableOnFinish = bEnableOnFinish;
		CurrentStart += StatItemInterval;
	};

	QueueItem(KillCountValueText2);
	QueueItem(KillCountValueText);
	QueueItem(SurvivalTimeValueText2);
	QueueItem(SurvivalTimeValueText);
	QueueItem(DamageTakenValueText2);
	QueueItem(DamageTakenValueText);
	QueueItem(RankValueText2);
	QueueItem(RankValueText);
	QueueItem(MainMenuButton, true);
}

void UClearResult::StartRevealSequence()
{
	RevealElapsedTime = 0.0f;

	if (RevealTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RevealTickerHandle);
		RevealTickerHandle.Reset();
	}

	RevealTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UClearResult::UpdateRevealSequence));
}

void UClearResult::StopRevealSequence()
{
	if (RevealTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RevealTickerHandle);
		RevealTickerHandle.Reset();
	}
}

bool UClearResult::UpdateRevealSequence(float DeltaTime)
{
	RevealElapsedTime += DeltaTime;

	if (Image_0)
	{
		// 배경은 항상 검은색 그대로 유지하고 페이드하지 않습니다.
		Image_0->SetRenderOpacity(1.0f);
		Image_0->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	auto AnimatePopShake = [this](UWidget* TargetWidget, const FWidgetTransform& BaseTransform, float StartTime, bool bUseStrongShake)
	{
		if (!TargetWidget)
		{
			return;
		}

		const float LocalTime = RevealElapsedTime - StartTime;
		if (LocalTime < 0.0f)
		{
			ApplyWidgetReveal(TargetWidget, 0.0f, ESlateVisibility::HitTestInvisible);
			return;
		}

		TargetWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		TargetWidget->SetRenderOpacity(1.0f);

		FWidgetTransform NewTransform = BaseTransform;

		if (LocalTime < TitlePopDuration)
		{
			const float Alpha = FMath::Clamp(LocalTime / TitlePopDuration, 0.0f, 1.0f);
			const float EaseOut = 1.0f - FMath::Pow(1.0f - Alpha, 3.0f);
			const float Scale = FMath::Lerp(0.25f, 1.14f, EaseOut);
			const float OffsetY = FMath::Lerp(36.0f, -3.0f, EaseOut);

			NewTransform.Scale = BaseTransform.Scale * Scale;
			NewTransform.Translation = BaseTransform.Translation + FVector2D(0.0f, OffsetY);
			TargetWidget->SetRenderTransform(NewTransform);
			return;
		}

		const float ShakeTime = LocalTime - TitlePopDuration;
		if (ShakeTime < TitleShakeDuration)
		{
			const float ShakeAlpha = FMath::Clamp(ShakeTime / TitleShakeDuration, 0.0f, 1.0f);
			const float Decay = 1.0f - ShakeAlpha;
			const float Strong = bUseStrongShake ? 1.0f : 0.65f;

			NewTransform.Scale = BaseTransform.Scale * FMath::Lerp(1.14f, 1.0f, ShakeAlpha);
			NewTransform.Translation = BaseTransform.Translation + FVector2D(
				FMath::Sin(ShakeAlpha * 4.0f * PI) * 10.0f * Decay * Strong,
				FMath::Sin(ShakeAlpha * 6.0f * PI) * 4.0f * Decay * Strong);
			NewTransform.Angle = BaseTransform.Angle + FMath::Sin(ShakeAlpha * 5.0f * PI) * 2.8f * Decay * Strong;
			TargetWidget->SetRenderTransform(NewTransform);
			return;
		}

		TargetWidget->SetRenderTransform(BaseTransform);
	};

	const float TitleStageStart = OverlayFadeOutDuration;
	AnimatePopShake(GameOverImage, GameOverImageBaseTransform, TitleStageStart, true);
	AnimatePopShake(GameOverImage2, GameOverImage2BaseTransform, TitleStageStart, true);

	constexpr float StatSettleDuration = 0.12f;
	for (FRevealItem& Item : RevealItems)
	{
		UWidget* TargetWidget = Item.Widget.Get();
		if (!TargetWidget)
		{
			continue;
		}

		const float LocalTime = RevealElapsedTime - Item.StartTime;
		if (LocalTime < 0.0f)
		{
			ApplyWidgetReveal(TargetWidget, 0.0f, Item.bEnableOnFinish ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
			continue;
		}

		const ESlateVisibility VisibleState = Item.bEnableOnFinish ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible;
		TargetWidget->SetVisibility(VisibleState);
		TargetWidget->SetRenderOpacity(1.0f);

		FWidgetTransform NewTransform = Item.BaseTransform;
		if (LocalTime < StatPopDuration)
		{
			const float Alpha = FMath::Clamp(LocalTime / StatPopDuration, 0.0f, 1.0f);
			const float EaseOut = 1.0f - FMath::Pow(1.0f - Alpha, 3.0f);
			NewTransform.Scale = Item.BaseTransform.Scale * FMath::Lerp(0.35f, 1.12f, EaseOut);
			NewTransform.Translation = Item.BaseTransform.Translation + FVector2D(0.0f, FMath::Lerp(20.0f, -2.0f, EaseOut));
			TargetWidget->SetRenderTransform(NewTransform);
			continue;
		}

		if (LocalTime < StatPopDuration + StatSettleDuration)
		{
			const float SettleAlpha = FMath::Clamp((LocalTime - StatPopDuration) / StatSettleDuration, 0.0f, 1.0f);
			NewTransform.Scale = Item.BaseTransform.Scale * FMath::Lerp(1.12f, 1.0f, SettleAlpha);
			NewTransform.Translation = Item.BaseTransform.Translation + FVector2D(
				FMath::Sin(SettleAlpha * PI) * 2.0f,
				FMath::Lerp(-2.0f, 0.0f, SettleAlpha));
			TargetWidget->SetRenderTransform(NewTransform);
			continue;
		}

		TargetWidget->SetRenderTransform(Item.BaseTransform);
		if (Item.bEnableOnFinish)
		{
			if (UButton* TargetButton = Cast<UButton>(TargetWidget))
			{
				TargetButton->SetIsEnabled(true);
			}
		}
	}

	const float StatsEndTime = RevealItems.IsEmpty()
		? (TitleStageStart + TitlePopDuration + TitleShakeDuration)
		: (RevealItems.Last().StartTime + StatPopDuration + StatSettleDuration);
	const float SequenceEnd = FMath::Max(StatsEndTime, TitleStageStart + TitlePopDuration + TitleShakeDuration);
	return RevealElapsedTime < SequenceEnd;
}

void UClearResult::ApplyWidgetReveal(UWidget* TargetWidget, float Alpha, ESlateVisibility VisibleState) const
{
	if (!TargetWidget)
	{
		return;
	}

	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	TargetWidget->SetVisibility(ClampedAlpha > 0.0f ? VisibleState : ESlateVisibility::Hidden);
	TargetWidget->SetRenderOpacity(ClampedAlpha);
}
