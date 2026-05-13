#include "ProjectTeam16/UI/BossHealthBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBossHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResetBossHealth();
	Hide();
}

void UBossHealthBarWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UBossHealthBarWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UBossHealthBarWidget::UpdateBossHealth(float CurrentHealth, float MaxHealth)
{
	const float BossHealthPercent = MaxHealth > 0.0f
		? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 0.0f;

	if (BossHealthProgressBar)
	{
		BossHealthProgressBar->SetPercent(BossHealthPercent);
	}

	if (BossHealthText)
	{
		const int32 CurrentHealthValue = FMath::Max(0, FMath::RoundToInt(CurrentHealth));
		const int32 MaxHealthValue = FMath::Max(0, FMath::RoundToInt(MaxHealth));
		BossHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHealthValue, MaxHealthValue)));
	}
}

void UBossHealthBarWidget::SetBossName(const FString& BossName)
{
	if (BossNameText)
	{
		BossNameText->SetText(FText::FromString(BossName));
	}
}

void UBossHealthBarWidget::ResetBossHealth()
{
	UpdateBossHealth(0.0f, 1.0f);
	SetBossName(TEXT("Boss"));
}

void UBossHealthBarWidget::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
	UpdateBossHealth(CurrentHealth, MaxHealth);
}
