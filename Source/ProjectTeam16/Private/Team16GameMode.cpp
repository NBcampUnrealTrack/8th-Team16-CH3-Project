#include "Team16GameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void ATeam16GameMode::BeginPlay()
{
    Super::BeginPlay();

    if (MainMenuWidgetClass)
    {
        MainMenuWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);

        if (MainMenuWidgetInstance)
        {
            MainMenuWidgetInstance->AddToViewport();

            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
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
}