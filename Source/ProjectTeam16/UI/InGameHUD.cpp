// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/InGameHUD.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UInGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (!BossHealthBox)
	{
		BossHealthBox = GetWidgetFromName(TEXT("BossHealthBox"));
	}

	if (!MiniMap)
	{
		MiniMap = GetWidgetFromName(TEXT("MiniMap"));
	}

	if (!PlayerIcon)
	{
		PlayerIcon = GetWidgetFromName(TEXT("PlayerIcon"));
	}

	if (!HpText)
	{
		HpText = Cast<UTextBlock>(GetWidgetFromName(TEXT("HpText")));
		if (!HpText)
		{
			HpText = Cast<UTextBlock>(GetWidgetFromName(TEXT("HPText")));
		}
	}

	if (!StaminaText)
	{
		StaminaText = Cast<UTextBlock>(GetWidgetFromName(TEXT("StaminaText")));
		if (!StaminaText)
		{
			StaminaText = Cast<UTextBlock>(GetWidgetFromName(TEXT("staminatext")));
		}
	}

	if (!BossnameText)
	{
		BossnameText = Cast<UTextBlock>(GetWidgetFromName(TEXT("BossNameText")));
	}

	if (!ZombieKillCountText)
	{
		ZombieKillCountText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ZombieKillCountText")));
	}

	if (!BossHealthText)
	{
		BossHealthText = Cast<UTextBlock>(GetWidgetFromName(TEXT("BossHPText")));
	}

	if (!BossHealthText)
	{
		BossHealthText = BossHPText;
	}

	ConfigureStableBossHealthText(BossHealthText);
	if (BossHPText && BossHPText != BossHealthText)
	{
		ConfigureStableBossHealthText(BossHPText);
	}

	// BP에서 HealthProgressBar 이름이 정확히 바인딩되면, 체력 바는 빨간색/100%로 시작합니다.
	if (HealthProgressBar)
	{
		HealthProgressBar->SetFillColorAndOpacity(FLinearColor::Red);
		HealthProgressBar->SetPercent(1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: HealthProgressBar is not bound. Check widget name and Is Variable."));
	}

	// BP에서 StaminaProgressBar 이름이 정확히 바인딩되면, 스태미나 바는 100%로 시작합니다.
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetFillColorAndOpacity(FLinearColor::Yellow);
		StaminaProgressBar->SetPercent(1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: StaminaProgressBar is not bound. Check widget name and Is Variable."));
	}

	// 경험치 바도 처음에는 0%에서 시작하도록 명시합니다.
	if (ExperienceProgressBar)
	{
		ExperienceProgressBar->SetPercent(0.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: ExperienceProgressBar is not bound. Check widget name and Is Variable."));
	}

	if (HpText)
	{
		HpText->SetText(FText::FromString(TEXT("100 / 100")));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: HpText/HPText is not bound. Check widget name and Is Variable."));
	}

	if (StaminaText)
	{
		StaminaText->SetText(FText::FromString(TEXT("100 / 100")));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: StaminaText is not bound. Check widget name and Is Variable."));
	}

	if (ZombieKillCountText)
	{
		ZombieKillCountText->SetText(FText::AsNumber(0));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: ZombieKillCountText is not bound. Check widget name and Is Variable."));
	}

	HideBossHealth();
}

void UInGameHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	TickBossIntroAnimation();
}

void UInGameHUD::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UInGameHUD::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInGameHUD::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	const float HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthPercent);
		HealthProgressBar->SetFillColorAndOpacity(FLinearColor::Red);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: HealthProgressBar is null. CurrentHealth=%f MaxHealth=%f Percent=%f"), CurrentHealth, MaxHealth, HealthPercent);
	}

	if (HpText)
	{
		const int32 CurrentHealthValue = FMath::Max(0, FMath::RoundToInt(CurrentHealth));
		const int32 MaxHealthValue = FMath::Max(0, FMath::RoundToInt(MaxHealth));
		HpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHealthValue, MaxHealthValue)));
	}
}

void UInGameHUD::UpdateStamina(float CurrentStamina, float MaxStamina)
{
	const float StaminaPercent = MaxStamina > 0.0f ? FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f) : 0.0f;

	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(StaminaPercent);
		StaminaProgressBar->SetFillColorAndOpacity(FLinearColor::Yellow);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: StaminaProgressBar is null. CurrentStamina=%f MaxStamina=%f Percent=%f"), CurrentStamina, MaxStamina, StaminaPercent);
	}

	if (StaminaText)
	{
		const int32 CurrentStaminaValue = FMath::Max(0, FMath::RoundToInt(CurrentStamina));
		const int32 MaxStaminaValue = FMath::Max(0, FMath::RoundToInt(MaxStamina));
		StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentStaminaValue, MaxStaminaValue)));
	}
}

void UInGameHUD::UpdateExperience(float CurrentExp, float MaxExp)
{
	const float ExpPercent = MaxExp > 0.0f ? FMath::Clamp(CurrentExp / MaxExp, 0.0f, 1.0f) : 0.0f;

	if (ExperienceProgressBar)
	{
		ExperienceProgressBar->SetPercent(ExpPercent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_InGameHUD: ExperienceProgressBar is null. CurrentExp=%f MaxExp=%f Percent=%f"), CurrentExp, MaxExp, ExpPercent);
	}
}

void UInGameHUD::UpdateLevel(int32 CurrentLevel)
{
	if (LevelText)
	{
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("LV %d"), CurrentLevel)));
	}
}

void UInGameHUD::UpdateZombieKillCount(int32 KillCount)
{
	if (ZombieKillCountText)
	{
		ZombieKillCountText->SetText(FText::AsNumber(KillCount));
	}
}

void UInGameHUD::UpdateTime(int32 RemainingSeconds)
{
	const int32 ClampedSeconds = FMath::Max(0, RemainingSeconds);
	const int32 Minutes = ClampedSeconds / 60;
	const int32 Seconds = ClampedSeconds % 60;

	if (TimeText)
	{
		TimeText->SetVisibility(ESlateVisibility::Visible);
		TimeText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
	}
}

void UInGameHUD::HideTime()
{
	if (TimeText)
	{
		TimeText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInGameHUD::ShowMiniMap()
{
	if (MiniMap)
	{
		MiniMap->SetVisibility(ESlateVisibility::Visible);
	}

	if (PlayerIcon)
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInGameHUD::HideMiniMap()
{
	if (MiniMap)
	{
		MiniMap->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (PlayerIcon)
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInGameHUD::ShowBossHealth(const FString& BossName, float CurrentHealth, float MaxHealth)
{
	HideMiniMap();
	StopBossIntroAnimation(true);

	if (BossHealthBox)
	{
		BossHealthBox->SetVisibility(ESlateVisibility::Visible);
	}

	if (BossHealthProgressBar)
	{
		BossHealthProgressBar->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BossHealthProgressBar_1)
	{
		BossHealthProgressBar_1->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BossHealthBlackbar)
	{
		BossHealthBlackbar->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BossnameText)
	{
		BossnameText->SetVisibility(ESlateVisibility::Hidden);
		BossnameText->SetText(FText::FromString(BossName));
	}

	if (BossHealthText)
	{
		BossHealthText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BossHPText)
	{
		BossHPText->SetVisibility(ESlateVisibility::Hidden);
	}

	UpdateBossHealth(CurrentHealth, MaxHealth);
	StartBossIntroAnimation();
}

void UInGameHUD::HideBossHealth()
{
	StopBossIntroAnimation(true);

	if (BossHealthBox)
	{
		BossHealthBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (BossHealthProgressBar)
	{
		BossHealthProgressBar->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (BossHealthProgressBar_1)
	{
		BossHealthProgressBar_1->SetVisibility(ESlateVisibility::Collapsed);
		BossHealthProgressBar_1->SetPercent(1.0f);
	}

	if (BossHealthBlackbar)
	{
		BossHealthBlackbar->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (BossnameText)
	{
		BossnameText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (BossHealthText)
	{
		BossHealthText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (BossHPText)
	{
		BossHPText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInGameHUD::UpdateBossHealth(float CurrentHealth, float MaxHealth)
{
	const float BossHealthPercent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;

	if (BossHealthProgressBar_1)
	{
		BossHealthProgressBar_1->SetPercent(BossHealthPercent);
	}

	if (BossHealthText)
	{
		const int32 CurrentHealthValue = FMath::Max(0, FMath::RoundToInt(CurrentHealth));
		const int32 MaxHealthValue = FMath::Max(0, FMath::RoundToInt(MaxHealth));
		BossHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHealthValue, MaxHealthValue)));
	}

	if (BossHPText && BossHPText != BossHealthText)
	{
		const int32 CurrentHealthValue = FMath::Max(0, FMath::RoundToInt(CurrentHealth));
		const int32 MaxHealthValue = FMath::Max(0, FMath::RoundToInt(MaxHealth));
		BossHPText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHealthValue, MaxHealthValue)));
	}
}

void UInGameHUD::ConfigureStableBossHealthText(UTextBlock* TargetText) const
{
	if (!TargetText)
	{
		return;
	}

	// 체력 숫자 자릿수가 바뀌어도 TextBlock의 원하는 크기가 줄어들지 않게 고정합니다.
	TargetText->SetAutoWrapText(false);
	TargetText->SetJustification(ETextJustify::Center);
	TargetText->SetMinDesiredWidth(260.0f);
}

void UInGameHUD::StartBossIntroAnimation()
{
	BossIntroAnimItems.Reset();
	QueueBossIntroWidget(BossnameText, 0.0f);

	// BossNameText만 먼저 나오고, 체력 텍스트와 바들은 같이 등장합니다.
	constexpr float BarGroupDelaySeconds = 0.7f;
	QueueBossIntroWidget(BossHealthText, BarGroupDelaySeconds);
	if (BossHPText && BossHPText != BossHealthText)
	{
		QueueBossIntroWidget(BossHPText, BarGroupDelaySeconds);
	}

	QueueBossIntroWidget(BossHealthProgressBar, BarGroupDelaySeconds);
	QueueBossIntroWidget(BossHealthProgressBar_1, BarGroupDelaySeconds);
	QueueBossIntroWidget(BossHealthBlackbar, BarGroupDelaySeconds);

	if (BossIntroAnimItems.IsEmpty())
	{
		bIsBossIntroAnimating = false;
		return;
	}

	UWorld* World = GetWorld();
	BossIntroStartWorldTime = World ? World->GetTimeSeconds() : 0.0f;
	bIsBossIntroAnimating = true;
}

void UInGameHUD::QueueBossIntroWidget(UWidget* TargetWidget, float StartDelaySeconds)
{
	if (!TargetWidget)
	{
		return;
	}

	FBossIntroAnimItem& Item = BossIntroAnimItems.Emplace_GetRef();
	Item.Widget = TargetWidget;
	Item.BaseTransform = TargetWidget->GetRenderTransform();
	Item.StartDelaySeconds = FMath::Max(0.0f, StartDelaySeconds);
	TargetWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UInGameHUD::TickBossIntroAnimation()
{
	if (!bIsBossIntroAnimating || BossIntroAnimItems.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Elapsed = FMath::Max(0.0f, World->GetTimeSeconds() - BossIntroStartWorldTime);
	constexpr float PopDurationSeconds = 0.23f;
	constexpr float ShakeDurationSeconds = 0.34f;
	constexpr float TotalDurationSeconds = PopDurationSeconds + ShakeDurationSeconds;
	bool bAllFinished = true;

	for (FBossIntroAnimItem& Item : BossIntroAnimItems)
	{
		if (Item.bFinished)
		{
			continue;
		}

		UWidget* TargetWidget = Item.Widget.Get();
		if (!TargetWidget)
		{
			Item.bFinished = true;
			continue;
		}

		if (Elapsed < Item.StartDelaySeconds)
		{
			bAllFinished = false;
			continue;
		}

		if (!Item.bStarted)
		{
			TargetWidget->SetVisibility(ESlateVisibility::Visible);
			Item.bStarted = true;
		}

		const float LocalTime = Elapsed - Item.StartDelaySeconds;
		FWidgetTransform NewTransform = Item.BaseTransform;
		float ScaleMultiplier = 1.0f;
		FVector2D TranslationOffset = FVector2D::ZeroVector;
		float AngleOffset = 0.0f;

		if (LocalTime < PopDurationSeconds)
		{
			bAllFinished = false;
			const float Alpha = LocalTime / PopDurationSeconds;
			const float EaseOut = 1.0f - FMath::Pow(1.0f - Alpha, 3.0f);
			ScaleMultiplier = FMath::Lerp(0.2f, 1.17f, EaseOut);
			TranslationOffset.Y = FMath::Lerp(30.0f, -4.0f, EaseOut);
		}
		else if (LocalTime < TotalDurationSeconds)
		{
			bAllFinished = false;
			const float ShakeAlpha = (LocalTime - PopDurationSeconds) / ShakeDurationSeconds;
			const float Decay = 1.0f - ShakeAlpha;
			ScaleMultiplier = FMath::Lerp(1.17f, 1.0f, ShakeAlpha);
			TranslationOffset.X = FMath::Sin(ShakeAlpha * 4.0f * PI) * 10.0f * Decay;
			TranslationOffset.Y = FMath::Sin(ShakeAlpha * 6.0f * PI) * 4.0f * Decay;
			AngleOffset = FMath::Sin(ShakeAlpha * 5.0f * PI) * 3.0f * Decay;
		}
		else
		{
			Item.bFinished = true;
		}

		if (Item.bFinished)
		{
			TargetWidget->SetRenderTransform(Item.BaseTransform);
			continue;
		}

		NewTransform.Scale = Item.BaseTransform.Scale * ScaleMultiplier;
		NewTransform.Translation = Item.BaseTransform.Translation + TranslationOffset;
		NewTransform.Angle = Item.BaseTransform.Angle + AngleOffset;
		TargetWidget->SetRenderTransform(NewTransform);
	}

	if (bAllFinished)
	{
		StopBossIntroAnimation(true);
	}
}

void UInGameHUD::StopBossIntroAnimation(bool bRestoreBaseTransform)
{
	if (bRestoreBaseTransform)
	{
		for (FBossIntroAnimItem& Item : BossIntroAnimItems)
		{
			if (UWidget* TargetWidget = Item.Widget.Get())
			{
				TargetWidget->SetRenderTransform(Item.BaseTransform);
			}
		}
	}

	BossIntroAnimItems.Reset();
	BossIntroStartWorldTime = 0.0f;
	bIsBossIntroAnimating = false;
}
