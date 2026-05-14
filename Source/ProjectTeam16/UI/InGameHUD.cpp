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

	if (!BossnameText)
	{
		BossnameText = Cast<UTextBlock>(GetWidgetFromName(TEXT("BossNameText")));
	}

	if (!BossHealthText)
	{
		BossHealthText = Cast<UTextBlock>(GetWidgetFromName(TEXT("BossHPText")));
	}

	if (!BossHealthText)
	{
		BossHealthText = BossHPText;
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

	HideBossHealth();
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
		HpText->SetText(FText::FromString(FString::Printf(TEXT("HP %d / %d"), CurrentHealthValue, MaxHealthValue)));
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
		ZombieKillCountText->SetText(FText::FromString(FString::Printf(TEXT("kill %d"), KillCount)));
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

void UInGameHUD::ShowBossHealth(const FString& BossName, float CurrentHealth, float MaxHealth)
{
	if (BossHealthBox)
	{
		BossHealthBox->SetVisibility(ESlateVisibility::Visible);
	}

	if (BossHealthProgressBar)
	{
		BossHealthProgressBar->SetVisibility(ESlateVisibility::Visible);
	}

	if (BossHealthProgressBar_1)
	{
		BossHealthProgressBar_1->SetVisibility(ESlateVisibility::Visible);
	}

	if (BossHealthBlackbar)
	{
		BossHealthBlackbar->SetVisibility(ESlateVisibility::Visible);
	}

	if (BossnameText)
	{
		BossnameText->SetVisibility(ESlateVisibility::Visible);
		BossnameText->SetText(FText::FromString(BossName));
	}

	if (BossHealthText)
	{
		BossHealthText->SetVisibility(ESlateVisibility::Visible);
	}

	if (BossHPText)
	{
		BossHPText->SetVisibility(ESlateVisibility::Visible);
	}

	UpdateBossHealth(CurrentHealth, MaxHealth);
}

void UInGameHUD::HideBossHealth()
{
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
