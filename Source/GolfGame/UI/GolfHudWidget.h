// GolfHudWidget.h
// Parent class for the always-on HUD (day, time, cash, phase). In your Widget Blueprint,
// bind each Text block's text to one of the Get...Text functions below; they read live values
// so no manual refresh is needed. OnStateChanged fires on phase/day/cash changes if you want
// to play an animation.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/GolfTypes.h"
#include "GolfHudWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class GOLFGAME_API UGolfHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetDayText() const;      // "Day: 1"

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetCashText() const;     // "Cash: $1000"

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetPhaseText() const;    // "Closed - Prep" / "Open" / "Closed"

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetTimeText() const;     // "Time: 5:00 AM"

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetCloseTimerText() const; // "Close Time: 0" (minutes into the Close phase)

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnStateChanged();

private:
	UFUNCTION()
	void HandlePhaseChanged(EDayPhase newPhase);

	UFUNCTION()
	void HandleDayEnded();

	UFUNCTION()
	void HandleCashChanged(int32 newCash);
};
