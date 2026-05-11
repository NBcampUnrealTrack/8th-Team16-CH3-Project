// Source/ProjectTeam16/UI/BossHealthBarWidget.cpp

#include "BossHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBossHealthBarWidget::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
    if (MaxHealth <= 0.0f) return;

    if (HealthProgressBar)
    {
        float Percent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
        HealthProgressBar->SetPercent(Percent);

        // 체력 비율에 따라 색상 변경
        FLinearColor BarColor;
        if (Percent > 0.7f)
            BarColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // 초록
        else if (Percent > 0.3f)
            BarColor = FLinearColor(1.0f, 0.7f, 0.0f, 1.0f); // 노랑
        else
            BarColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨강

        HealthProgressBar->SetFillColorAndOpacity(BarColor);
    }

    if (HealthText)
    {
        FString HealthString = FString::Printf(
            TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth);
        HealthText->SetText(FText::FromString(HealthString));
    }
}

void UBossHealthBarWidget::SetBossName(const FString& Name)
{
    if (BossNameText)
    {
        BossNameText->SetText(FText::FromString(Name));
    }
}

// 보스 캐릭터에서 호출하는 방법
// HealthBarComponent->SetWidgetClass(WBP_BossHealthBar);
// Cast<UBossHealthBarWidget>(HealthBarComponent->GetWidget())->UpdateHealthBar(CurrentHealth, MaxHealth);
// Cast<UBossHealthBarWidget>(HealthBarComponent->GetWidget())->SetBossName(TEXT("Boss"));