// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectTeam16/UI/MainMenu.h"
#include "Team16PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Containers/Ticker.h"

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::OnStartButtonClicked);
		StartButton->OnHovered.AddUniqueDynamic(this, &UMainMenu::OnStartButtonHovered);
		StartButton->OnUnhovered.AddUniqueDynamic(this, &UMainMenu::OnStartButtonUnhovered);
	}

	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::OnOptionsButtonClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UMainMenu::OnQuitButtonClicked);
		QuitButton->OnHovered.AddUniqueDynamic(this, &UMainMenu::OnQuitButtonHovered);
		QuitButton->OnUnhovered.AddUniqueDynamic(this, &UMainMenu::OnQuitButtonUnhovered);
	}

	SetButtonImageHovered(StartButtonImage, false);
	SetButtonImageHovered(QuitButtonImage, false);
	ResolveBackgroundLightsOffImage();
	StartMainMenuLightFlicker();
}

void UMainMenu::NativeDestruct()
{
	StopMainMenuLightFlicker();

	Super::NativeDestruct();
}

void UMainMenu::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	StartMainMenuLightFlicker();
}

void UMainMenu::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
	StopMainMenuLightFlicker();
}

void UMainMenu::OnStartButtonClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	RemoveFromParent();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(PC))
		{
			Team16PlayerController->ShowInGameHUD();
			Team16PlayerController->PlayFadeOut();
		}
	}
}

void UMainMenu::OnStartButtonHovered()
{
	SetButtonImageHovered(StartButtonImage, true);
}

void UMainMenu::OnStartButtonUnhovered()
{
	SetButtonImageHovered(StartButtonImage, false);
}

void UMainMenu::OnOptionsButtonClicked()
{
}

void UMainMenu::OnQuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

void UMainMenu::OnQuitButtonHovered()
{
	SetButtonImageHovered(QuitButtonImage, true);
}

void UMainMenu::OnQuitButtonUnhovered()
{
	SetButtonImageHovered(QuitButtonImage, false);
}

void UMainMenu::SetButtonImageHovered(UImage* ButtonImage, bool bIsHovered) const
{
	if (!ButtonImage)
	{
		return;
	}

	ButtonImage->SetColorAndOpacity(bIsHovered ? ButtonImageHoveredTint : ButtonImageNormalTint);
}

void UMainMenu::ResolveBackgroundLightsOffImage()
{
	if (BackgroundVideoImage_1)
	{
		return;
	}

	if (!WidgetTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: WidgetTree is null. Cannot resolve BackgroundVideoImage_1."));
		return;
	}

	static const FName CandidateNames[] = {
		TEXT("BackgroundVideoImage_1"),
		TEXT("BackgroundImage_1"),
		TEXT("BackgroundVideoImage1"),
		TEXT("BackgroundImage1")
	};

	for (const FName CandidateName : CandidateNames)
	{
		if (UImage* FoundImage = Cast<UImage>(WidgetTree->FindWidget(CandidateName)))
		{
			BackgroundVideoImage_1 = FoundImage;
			break;
		}
	}

	if (!BackgroundVideoImage_1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: BackgroundVideoImage_1 not found. Flicker is disabled. Check widget name and Is Variable."));
		return;
	}

	BackgroundVideoImage_1->SetVisibility(ESlateVisibility::Hidden);
	BackgroundVideoImage_1->SetRenderOpacity(1.0f);
	BackgroundVideoImage_1->SetColorAndOpacity(FLinearColor::White);

	// Ensure the lights-off image is rendered above the base background image.
	if (UCanvasPanelSlot* LightsOffSlot = Cast<UCanvasPanelSlot>(BackgroundVideoImage_1->Slot))
	{
		if (UCanvasPanelSlot* BaseSlot = BackgroundVideoImage ? Cast<UCanvasPanelSlot>(BackgroundVideoImage->Slot) : nullptr)
		{
			LightsOffSlot->SetZOrder(BaseSlot->GetZOrder() + 1);
		}
	}
}

void UMainMenu::StartMainMenuLightFlicker()
{
	if (!bEnableMainMenuLightFlicker || !BackgroundVideoImage_1)
	{
		if (!BackgroundVideoImage_1)
		{
			UE_LOG(LogTemp, Warning, TEXT("MainMenu: BackgroundVideoImage_1 is null. Flicker did not start."));
		}
		return;
	}

	if (MainMenuLightFlickerTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(MainMenuLightFlickerTickerHandle);
		MainMenuLightFlickerTickerHandle.Reset();
	}

	bIsMainMenuLightFlickerRunning = false;
	HideMainMenuLightsOffFrame();
	bIsMainMenuLightFlickerRunning = true;
	bIsLightsOffFrameVisible = false;
	CurrentFlickerPhaseElapsed = 0.0f;
	ScheduleNextMainMenuLightFlicker();

	MainMenuLightFlickerTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UMainMenu::UpdateMainMenuLightFlicker));
}

void UMainMenu::StopMainMenuLightFlicker()
{
	bIsMainMenuLightFlickerRunning = false;

	if (MainMenuLightFlickerTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(MainMenuLightFlickerTickerHandle);
		MainMenuLightFlickerTickerHandle.Reset();
	}

	HideMainMenuLightsOffFrame();
}

void UMainMenu::ScheduleNextMainMenuLightFlicker()
{
	if (!bIsMainMenuLightFlickerRunning || !bEnableMainMenuLightFlicker || !BackgroundVideoImage_1 || GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}

	const float MinDelay = FMath::Max(0.01f, LightFlickerMinDelay);
	const float MaxDelay = FMath::Max(MinDelay, LightFlickerMaxDelay);
	CurrentFlickerPhaseDuration = FMath::FRandRange(MinDelay, MaxDelay);
	CurrentFlickerPhaseElapsed = 0.0f;
}

void UMainMenu::ShowMainMenuLightsOffFrame()
{
	if (!bIsMainMenuLightFlickerRunning || !bEnableMainMenuLightFlicker || !BackgroundVideoImage_1 || GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}

	BackgroundVideoImage_1->SetRenderOpacity(LightFlickerLightsOffOpacity);
	BackgroundVideoImage_1->SetColorAndOpacity(FLinearColor::White);
	BackgroundVideoImage_1->SetVisibility(ESlateVisibility::Visible);
	bIsLightsOffFrameVisible = true;

	const float MinDuration = FMath::Max(0.01f, LightFlickerMinDuration);
	const float MaxDuration = FMath::Max(MinDuration, LightFlickerMaxDuration);
	CurrentFlickerPhaseDuration = FMath::FRandRange(MinDuration, MaxDuration);
	CurrentFlickerPhaseElapsed = 0.0f;
}

void UMainMenu::HideMainMenuLightsOffFrame()
{
	if (BackgroundVideoImage_1)
	{
		BackgroundVideoImage_1->SetRenderOpacity(1.0f);
		BackgroundVideoImage_1->SetColorAndOpacity(FLinearColor::White);
		BackgroundVideoImage_1->SetVisibility(ESlateVisibility::Hidden);
	}
	bIsLightsOffFrameVisible = false;

	if (bIsMainMenuLightFlickerRunning)
	{
		ScheduleNextMainMenuLightFlicker();
	}
}

bool UMainMenu::UpdateMainMenuLightFlicker(float DeltaTime)
{
	if (!bIsMainMenuLightFlickerRunning)
	{
		return false;
	}

	if (!BackgroundVideoImage_1 || GetVisibility() == ESlateVisibility::Collapsed)
	{
		return true;
	}

	CurrentFlickerPhaseElapsed += DeltaTime;
	if (CurrentFlickerPhaseElapsed < CurrentFlickerPhaseDuration)
	{
		return true;
	}

	if (bIsLightsOffFrameVisible)
	{
		HideMainMenuLightsOffFrame();
	}
	else
	{
		ShowMainMenuLightsOffFrame();
	}

	return true;
}
