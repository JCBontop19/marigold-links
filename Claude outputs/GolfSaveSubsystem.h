// GolfSaveSubsystem.h
// Save and load for all the game-instance systems. Autosaves to the "Auto" slot at the start of
// every day (the cut mask is empty then, so nothing is lost). Manual saves go to any slot name.
//
// After a load it fires onGameLoaded. Anything that only updates on events (HUD, sun, furniture
// actors, doors) should bind to it and refresh from the systems.
//
// Console: golf.Save [slot], golf.Load [slot]

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GolfSaveSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSaved, FString, slotName);

UCLASS()
class GOLFGAME_API UGolfSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnGameLoaded onGameLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnGameSaved onGameSaved;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveToSlot(const FString& slotName = TEXT("Slot1"));

	// Returns false (and changes nothing) if the slot is missing, unreadable, or from a newer version.
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadFromSlot(const FString& slotName = TEXT("Slot1"));

	UFUNCTION(BlueprintPure, Category = "Save")
	bool DoesSaveExist(const FString& slotName = TEXT("Slot1")) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSave(const FString& slotName = TEXT("Slot1"));

	// For a load menu: day, cash and the local save time of a slot. False if the slot is missing.
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool GetSaveInfo(const FString& slotName, int32& day, int32& cash, FString& savedAtLocal) const;

private:
	UFUNCTION()
	void HandleDayStarted(int32 newDay);
};
