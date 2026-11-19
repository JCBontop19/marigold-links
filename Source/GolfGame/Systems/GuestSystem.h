// GuestSystem.h
// Guests and satisfaction. Replaces the flat per-hole revenue placeholder.
//
// How a day works (run once by DayCycle at the start of Close processing, BEFORE staff maintenance
// and natural decay, so guests play on the course as the player left it):
//
//   satisfaction (per hole)  = fairway*0.35 + rough*0.15 + green*0.50   (each condition 0..1)
//                              minus hazard penalties (UHazardSystem: unraked bunker, dirty pond)
//   arrivals (per hole)      = baseGuestsPerHole * lerp(0.5, 1.5, reputation)
//   paying guests (per hole) = round(arrivals * lerp(minPlayFraction, 1, satisfaction))
//   revenue                  = paying guests * greenFee
//
// Reputation (0..1, starts 0.5) drifts toward the day's average satisfaction by reputationRate
// every day. So a well-kept course slowly draws more guests, and a neglected one loses them.
// At reputation 0.5 and perfect conditions a hole earns 200, the same as the old placeholder.
//
// All numbers below are first-pass values: tune them by feel.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GolfTypes.h"
#include "GuestSystem.generated.h"

class UCourseManager;
class UGolfSaveGame;

UCLASS()
class GOLFGAME_API UGuestSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Tuning ---
	static constexpr float fairwayWeight = 0.35f;
	static constexpr float roughWeight = 0.15f;
	static constexpr float greenWeight = 0.50f;
	static constexpr int32 baseGuestsPerHole = 8;
	static constexpr int32 greenFee = 25;
	static constexpr float minPlayFraction = 0.25f;        // even a neglected hole keeps some diehards
	static constexpr float minArrivalMultiplier = 0.5f;    // at reputation 0
	static constexpr float maxArrivalMultiplier = 1.5f;    // at reputation 1
	static constexpr float startingReputation = 0.5f;
	static constexpr float reputationRate = 0.2f;          // how fast reputation follows satisfaction

	// 0..1, how happy guests are with this hole right now. 0 for an unknown hole.
	UFUNCTION(BlueprintPure, Category = "Guests")
	float GetHoleSatisfaction(int32 holeNumber) const;

	UFUNCTION(BlueprintPure, Category = "Guests")
	float GetReputation() const { return reputation; }

	// lerp(0.5, 1.5, reputation)
	UFUNCTION(BlueprintPure, Category = "Guests")
	float GetArrivalMultiplier() const;

	// Paying guests this hole would draw if the day closed right now (for UI previews).
	UFUNCTION(BlueprintPure, Category = "Guests")
	int32 GetExpectedGuests(int32 holeNumber) const;

	// Result of the most recent day (zeros before the first close).
	UFUNCTION(BlueprintPure, Category = "Guests")
	FGuestDayResult GetLastResult() const { return lastResult; }

	// Runs the day: counts guests and revenue over every unlocked revenue hole, then moves
	// reputation. Called by DayCycle; changes reputation, so call it once per day.
	UFUNCTION(BlueprintCallable, Category = "Guests")
	FGuestDayResult ProcessDay();

	// Console: golf.SetReputation <0..1>
	UFUNCTION(BlueprintCallable, Category = "Guests|Debug")
	void DebugSetReputation(float newReputation);

	// Save and load (called by UGolfSaveSubsystem).
	void WriteToSave(UGolfSaveGame& save) const;
	void ReadFromSave(const UGolfSaveGame& save);

private:
	UCourseManager* GetCourse() const;
	int32 GuestsForSatisfaction(float satisfaction) const;

	UPROPERTY()
	float reputation = startingReputation;

	UPROPERTY()
	FGuestDayResult lastResult;
};
