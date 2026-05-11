// BossHealthBarWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHealthBarWidget.generated.h"

UCLASS()
class PROJECTTEAM16_API UBossHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 체력바 업데이트 (보스 캐릭터에서 호출)
    UFUNCTION(BlueprintCallable)
    void UpdateHealthBar(float CurrentHealth, float MaxHealth);

    // 보스 이름 설정
    UFUNCTION(BlueprintCallable)
    void SetBossName(const FString& Name);

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* HealthProgressBar;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* BossNameText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* HealthText;
};