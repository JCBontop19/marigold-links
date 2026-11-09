// CourseCoverageSubsystem.h
// Turns hands-on mowing into hole condition.
//
// At the start of the Close phase it reads the cut-mask render target once, measures how much of
// each HoleCoverageZone is mowed, and calls CourseManager::ApplyConditionChange. A fully mowed
// zone restores (restore-per-mow) of its stat; half mowed restores half. The restore amount comes
// from the equipment the player owns, so mower tiers matter.
//
// It does nothing if no zones are registered or the cut mask was never configured, so the
// GolfSystemTest level (no zones) behaves exactly as before.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/GolfTypes.h"
#include "CourseCoverageSubsystem.generated.h"

class AHoleCoverageZone;
class UTextureRenderTarget2D;

UCLASS()
class GOLFGAME_API UCourseCoverageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* outer) const override;
	virtual void OnWorldBeginPlay(UWorld& inWorld) override;
	virtual void Deinitialize() override;

	// Call once at BeginPlay (BP_DayNightLighting is a good spot, it already holds the cut mask).
	// mowOrigin and mowSize must match MPC_Course: (-5000, -5000) and 63000.
	UFUNCTION(BlueprintCallable, Category = "Coverage")
	void Configure(UTextureRenderTarget2D* inCutMask, FVector2D inMowOrigin, float inMowSize);

	void RegisterZone(AHoleCoverageZone* zone);
	void UnregisterZone(AHoleCoverageZone* zone);

	// Measures every zone and applies the condition changes. Runs automatically when the Close
	// phase starts (once per day). Returns how many zones were measured. Pass force = true to
	// run it again the same day (handy for testing from a Blueprint key press).
	UFUNCTION(BlueprintCallable, Category = "Coverage")
	int32 ApplyCoverageToCourse(bool force = false);

	// Coverage (0-1) measured at the last close, for UI and debugging. 0 if never measured.
	UFUNCTION(BlueprintPure, Category = "Coverage")
	float GetLastCoverage(int32 holeNumber, EConditionStat stat) const;

private:
	UFUNCTION()
	void HandlePhaseChanged(EDayPhase newPhase);

	// Restore applied to a stat for a fully mowed zone, based on equipment the player owns.
	float GetRestorePerFullMow(EConditionStat stat, int32 ownedTier) const;

	static int32 MakeKey(int32 holeNumber, EConditionStat stat)
	{
		return holeNumber * 10 + static_cast<int32>(stat);
	}

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> cutMask;

	FVector2D mowOrigin = FVector2D(-5000.0, -5000.0);
	float mowSize = 63000.0f;

	TArray<TWeakObjectPtr<AHoleCoverageZone>> zones;
	TMap<int32, float> lastCoverage;
	int32 lastAppliedDay = -1;
};
