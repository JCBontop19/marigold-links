// HazardSystem.h
// Sand bunkers and water ponds that need upkeep, like the grass does.
//
//   Sand:  each new day an unlocked hole with a sand hazard has a chance to need raking
//          (a yes/no flag). Raking clears it.
//   Water: each new day an unlocked hole with a water hazard loses a little water condition
//          (1 = clean, 0 = foul). Cleaning sets it back to 1.
//
// Both pull the hole's guest satisfaction down while they are neglected (UGuestSystem reads
// GetSatisfactionPenalty). The player rakes and cleans from the edge with Press E; the interact
// spots are Blueprints that call RakeSand / CleanWater.
//
// DayCycle calls StartNewDay() when a day is closed out, BEFORE the new day's autosave, so a
// loaded autosave already contains that day's roll. Day 1 starts clean.
//
// All numbers are first-pass values: tune them by feel.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GolfTypes.h"
#include "HazardSystem.generated.h"

class UCourseManager;
class UGolfSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardChanged, int32, holeNumber);

UCLASS()
class GOLFGAME_API UHazardSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Tuning ---
	static constexpr float sandMessChance = 0.5f;           // per hole, per new day
	static constexpr float waterDecayPerDay = 0.04f;        // slow: clean to the 0.7 "needs attention" line in about a week
	static constexpr float sandPenalty = 0.10f;             // satisfaction lost while a bunker is unraked
	static constexpr float waterPenaltyMax = 0.15f;         // satisfaction lost when the water is fully foul
	static constexpr float waterAttentionThreshold = 0.7f;  // below this, the pond counts as "needs attention"

	// Fires when a hole's hazard changes (rolled, raked, cleaned, loaded).
	UPROPERTY(BlueprintAssignable, Category = "Hazards")
	FOnHazardChanged onHazardChanged;

	UFUNCTION(BlueprintPure, Category = "Hazards")
	bool HasSand(int32 holeNumber) const;

	UFUNCTION(BlueprintPure, Category = "Hazards")
	bool HasWater(int32 holeNumber) const;

	UFUNCTION(BlueprintPure, Category = "Hazards")
	bool NeedsRaking(int32 holeNumber) const;

	// 0..1, 1 = clean. 1 for a hole without water.
	UFUNCTION(BlueprintPure, Category = "Hazards")
	float GetWaterCondition(int32 holeNumber) const;

	// Satisfaction this hole loses to its hazards right now (0 when everything is in order).
	UFUNCTION(BlueprintPure, Category = "Hazards")
	float GetSatisfactionPenalty(int32 holeNumber) const;

	// Number of hazards on unlocked holes that need the player (unraked bunkers + dirty ponds), for a HUD.
	UFUNCTION(BlueprintPure, Category = "Hazards")
	int32 GetAttentionCount() const;

	// True if the bunker needed raking (and now does not).
	UFUNCTION(BlueprintCallable, Category = "Hazards")
	bool RakeSand(int32 holeNumber);

	// True if the pond was not already clean (and now is).
	UFUNCTION(BlueprintCallable, Category = "Hazards")
	bool CleanWater(int32 holeNumber);

	// Called by DayCycle once per new day.
	void StartNewDay();

	// Console: golf.MessHazards (every hazard hole: bunker unraked, water 0.3), golf.HazardStatus
	UFUNCTION(BlueprintCallable, Category = "Hazards|Debug")
	void DebugMessAll();

	UFUNCTION(BlueprintCallable, Category = "Hazards|Debug")
	void DebugSetSandNeedsRaking(int32 holeNumber, bool needsRaking);

	UFUNCTION(BlueprintCallable, Category = "Hazards|Debug")
	void DebugSetWater(int32 holeNumber, float waterCondition);

	void LogStatus() const;

	// Save and load (called by UGolfSaveSubsystem). Loading resets every hazard to clean first,
	// so an older save without hazard data comes back clean.
	void WriteToSave(UGolfSaveGame& save) const;
	void ReadFromSave(const UGolfSaveGame& save);

private:
	UCourseManager* GetCourse() const;
	FHazardState* FindState(int32 holeNumber);
	const FHazardState* FindState(int32 holeNumber) const;

	UPROPERTY()
	TArray<FHazardState> states;
};
