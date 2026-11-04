// GolfPanelBase.h
// Parent class for every full-screen panel (report, applications, hole unlock, hardware store).
// Design the layout in a Widget Blueprint that uses this as its parent class. All game logic
// the panels need is exposed here as Blueprint-callable helpers.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GolfPanelBase.generated.h"

class UCourseManager;
class UStaffSystem;
class UDayCycleSubsystem;
class UClubhouseSystem;
class UHoleData;
class UEquipmentData;
class UStaffData;
class UApplicationPairData;

UCLASS(Abstract, Blueprintable)
class GOLFGAME_API UGolfPanelBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called by the UI manager right after the panel is created.
	void InitPanel(UObject* inContext);

	// Blueprint hook: populate your lists/labels here. context is whatever the opener passed
	// (e.g. an ApplicationPairData for the applications desk), or null.
	UFUNCTION(BlueprintImplementableEvent, Category = "Panel")
	void OnPanelOpened(UObject* context);

	UPROPERTY(BlueprintReadOnly, Category = "Panel")
	TObjectPtr<UObject> panelContext;

	// Closes this panel (unpauses, restores the HUD and game input).
	UFUNCTION(BlueprintCallable, Category = "Panel")
	void ClosePanel();

	// ---- Subsystem access
	UFUNCTION(BlueprintPure, Category = "Golf")
	UCourseManager* GetCourseManager() const;

	UFUNCTION(BlueprintPure, Category = "Golf")
	UStaffSystem* GetStaffSystem() const;

	UFUNCTION(BlueprintPure, Category = "Golf")
	UDayCycleSubsystem* GetDayCycle() const;

	UFUNCTION(BlueprintPure, Category = "Golf")
	UClubhouseSystem* GetClubhouseSystem() const;

	// ---- Panel-specific helpers
	// Holes still locked that should appear in the expansion list (skips hub/clubhouse areas).
	UFUNCTION(BlueprintPure, Category = "Golf|Panels")
	TArray<UHoleData*> GetLockedHoles() const;

	// Every equipment item in the registry, lowest tier first.
	UFUNCTION(BlueprintPure, Category = "Golf|Panels")
	TArray<UEquipmentData*> GetEquipmentForSale() const;

	// Applications desk: hires the chosen applicant, rejects the other, closes the panel.
	UFUNCTION(BlueprintCallable, Category = "Golf|Panels")
	void ResolveApplication(UApplicationPairData* pair, bool hireA);

	// Report desk: closes the panel and ends the day (starts the next one).
	UFUNCTION(BlueprintCallable, Category = "Golf|Panels")
	void CloseReportAndEndDay();
};
