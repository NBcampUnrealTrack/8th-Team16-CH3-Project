// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Team16PlayerController.generated.h"

class UInputAction;
class UUserWidget;

UCLASS()
class PROJECTTEAM16_API ATeam16PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidgetInstance;
};
