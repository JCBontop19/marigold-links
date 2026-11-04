// GolfUIManager.h
// Opens/closes one full-screen panel at a time. While a panel is open the game is paused
// (the day clock stops), the HUD is hidden and input switches to the UI with a mouse cursor.
// Mirrors the Godot panels: paused tree + Hud.visible = false.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GolfUIManager.generated.h"

class UGolfPanelBase;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPanelVisibilityChanged);

UCLASS()
class GOLFGAME_API UGolfUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnPanelVisibilityChanged onPanelOpened;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnPanelVisibilityChanged onPanelClosed;

	// Returns the created panel, or null if one is already open / creation failed.
	UFUNCTION(BlueprintCallable, Category = "UI")
	UGolfPanelBase* ShowPanel(TSubclassOf<UGolfPanelBase> panelClass, UObject* context);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseActivePanel();

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsPanelOpen() const { return activePanel != nullptr; }

	// The HUD widget registers itself so it can be hidden while a panel is open.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RegisterHud(UUserWidget* hudWidget);

private:
	void SetHudVisible(bool visible);

	UPROPERTY()
	TObjectPtr<UGolfPanelBase> activePanel;

	TWeakObjectPtr<UUserWidget> hud;
};
