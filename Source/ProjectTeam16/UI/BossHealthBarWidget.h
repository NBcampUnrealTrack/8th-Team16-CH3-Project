#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTTEAM16_API UBossHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Boss UI")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "Boss UI")
	void Hide();

	UFUNCTION(BlueprintCallable, Category = "Boss UI")
	void UpdateBossHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "Boss UI")
	void SetBossName(const FString& BossName);

	UFUNCTION(BlueprintCallable, Category = "Boss UI")
	void ResetBossHealth();

	// 기존 호출부가 있어도 깨지지 않게 남겨둔 호환 함수입니다.
	UFUNCTION(BlueprintCallable, Category = "Boss UI")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> BossHealthProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossHealthText;
};
