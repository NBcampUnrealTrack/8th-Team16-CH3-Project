#include "Team16GameMode.h"
#include "Team16PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/UI/MainMenu.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATeam16GameMode::ATeam16GameMode()
{
    PlayerControllerClass = ATeam16PlayerController::StaticClass();
    DefaultPawnClass = ASP_Character::StaticClass();

    static ConstructorHelpers::FClassFinder<UMainMenu> DefaultMainMenuWidgetClass(TEXT("/Game/UI/Menus/WBP_MainMenu"));
    if (DefaultMainMenuWidgetClass.Succeeded())
    {
        MainMenuWidgetClass = DefaultMainMenuWidgetClass.Class;
    }
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
            if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(PC))
            {
                Team16PlayerController->ShowInGameHUD();
            }

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
        UE_LOG(LogTemp, Warning, TEXT("MainMenu: Missing World or MainMenuWidgetClass. World=%s Class=%s"),
            World ? TEXT("Valid") : TEXT("Null"),
            MainMenuWidgetClass ? *MainMenuWidgetClass->GetName() : TEXT("Null"));
        return;
    }

    if (!MainMenuWidgetInstance)
    {
        MainMenuWidgetInstance = CreateWidget<UMainMenu>(World, MainMenuWidgetClass);
        UE_LOG(LogTemp, Log, TEXT("MainMenu: Created widget instance from class %s. Instance=%s"),
            *MainMenuWidgetClass->GetName(),
            MainMenuWidgetInstance ? *MainMenuWidgetInstance->GetClass()->GetName() : TEXT("Null"));
    }

    if (MainMenuWidgetInstance)
    {
        if (!MainMenuWidgetInstance->IsInViewport())
        {
            MainMenuWidgetInstance->AddToViewport();
        }

        MainMenuWidgetInstance->Show();

        UGameplayStatics::SetGamePaused(World, true);

        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC)
        {
            if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(PC))
            {
                Team16PlayerController->HideInGameHUD();
                Team16PlayerController->PlayMainMenuBGM();
            }

            PC->bShowMouseCursor = true;

            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PC->SetInputMode(InputMode);
        }
    }
}
