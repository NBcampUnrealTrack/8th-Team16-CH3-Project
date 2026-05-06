#include "Team16GameMode.h"
#include "Team16PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

ATeam16GameMode::ATeam16GameMode()
{
    PlayerControllerClass = ATeam16PlayerController::StaticClass();
}

void ATeam16GameMode::BeginPlay()
{
    Super::BeginPlay();

    if (UGameplayStatics::HasOption(OptionsString, TEXT("SkipMainMenu")))
    {
        UWorld* World = GetWorld();
        UGameplayStatics::SetGamePaused(World, false);

        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC)
        {
            PC->bShowMouseCursor = false;

            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
        }

        return;
    }

    ShowMainMenu();
}

void ATeam16GameMode::ShowMainMenu()
{
    UWorld* World = GetWorld();
    if (!World || !MainMenuWidgetClass)
    {
        return;
    }

    if (!MainMenuWidgetInstance)
    {
        MainMenuWidgetInstance = CreateWidget<UUserWidget>(World, MainMenuWidgetClass);
    }

    if (MainMenuWidgetInstance)
    {
        if (!MainMenuWidgetInstance->IsInViewport())
        {
            MainMenuWidgetInstance->AddToViewport();
        }

        UGameplayStatics::SetGamePaused(World, true);

        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC)
        {
            PC->bShowMouseCursor = true;

            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PC->SetInputMode(InputMode);
        }
    }
}
