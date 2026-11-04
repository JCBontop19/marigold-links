#include "UI/GolfUIManager.h"
#include "UI/GolfPanelBase.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UGolfPanelBase* UGolfUIManager::ShowPanel(TSubclassOf<UGolfPanelBase> panelClass, UObject* context)
{
	if (activePanel || !panelClass.Get())
	{
		return nullptr;
	}

	UWorld* world = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	APlayerController* controller = world ? UGameplayStatics::GetPlayerController(world, 0) : nullptr;
	if (!controller)
	{
		return nullptr;
	}

	UGolfPanelBase* panel = CreateWidget<UGolfPanelBase>(controller, panelClass);
	if (!panel)
	{
		return nullptr;
	}

	activePanel = panel;
	panel->AddToViewport(10);
	panel->InitPanel(context);

	SetHudVisible(false);

	FInputModeUIOnly inputMode;
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	controller->SetInputMode(inputMode);
	controller->bShowMouseCursor = true;
	UGameplayStatics::SetGamePaused(world, true);

	onPanelOpened.Broadcast();
	return panel;
}

void UGolfUIManager::CloseActivePanel()
{
	if (!activePanel)
	{
		return;
	}

	activePanel->RemoveFromParent();
	activePanel = nullptr;

	UWorld* world = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (world)
	{
		UGameplayStatics::SetGamePaused(world, false);
		if (APlayerController* controller = UGameplayStatics::GetPlayerController(world, 0))
		{
			controller->SetInputMode(FInputModeGameOnly());
			controller->bShowMouseCursor = false;
		}
	}

	SetHudVisible(true);
	onPanelClosed.Broadcast();
}

void UGolfUIManager::RegisterHud(UUserWidget* hudWidget)
{
	hud = hudWidget;
}

void UGolfUIManager::SetHudVisible(bool visible)
{
	if (UUserWidget* hudWidget = hud.Get())
	{
		hudWidget->SetVisibility(visible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
