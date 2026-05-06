// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Team16GameMode.generated.h"

class UUserWidget;

UCLASS()
class PROJECTTEAM16_API ATeam16GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ATeam16GameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY()
	UUserWidget* MainMenuWidgetInstance;
};
