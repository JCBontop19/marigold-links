// CourseManager.h
// Port of the CourseManager autoload. Owns cash, equipment tier, and per-hole runtime state.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GolfTypes.h"
#include "CourseManager.generated.h"

class UHoleData;
class UEquipmentData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCashChanged, int32, newCash);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoleUnlocked, int32, holeNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentTierChanged, int32, newTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHoleConditionChanged, int32, holeNumber, EConditionStat, stat);

UCLASS()
class GOLFGAME_API UCourseManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static constexpr int32 startingCash = 1000;

	// --- Events (HUD and panels bind to these instead of polling) ---
	UPROPERTY(BlueprintAssignable, Category = "Course")
	FOnCashChanged onCashChanged;

	UPROPERTY(BlueprintAssignable, Category = "Course")
	FOnHoleUnlocked onHoleUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Course")
	FOnEquipmentTierChanged onEquipmentTierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Course")
	FOnHoleConditionChanged onHoleConditionChanged;

	// --- Cash ---
	UFUNCTION(BlueprintPure, Category = "Course")
	int32 GetPlayerCash() const { return playerCash; }

	UFUNCTION(BlueprintCallable, Category = "Course")
	void AddCash(int32 amount);

	// Fails (returns false, changes nothing) if the player cannot afford it.
	UFUNCTION(BlueprintCallable, Category = "Course")
	bool TrySpendCash(int32 amount);

	// --- Equipment ---
	UFUNCTION(BlueprintPure, Category = "Course")
	int32 GetOwnedEquipmentTier() const { return ownedEquipmentTier; }

	// Fails if the tier is already owned (or lower) or the player cannot afford it.
	UFUNCTION(BlueprintCallable, Category = "Course")
	bool PurchaseEquipment(UEquipmentData* equipment);

	// The equipment data asset for the tier the player owns right now (null if none matches).
	// The mower reads cutRadiusCm and moveSpeedMultiplier from this.
	UFUNCTION(BlueprintPure, Category = "Course")
	UEquipmentData* GetOwnedEquipmentData() const;

	// Debug and testing: set the owned tier directly (no cost) and fire the change event.
	// Console: golf.SetTier <n>
	UFUNCTION(BlueprintCallable, Category = "Course|Debug")
	void DebugSetEquipmentTier(int32 newTier);

	// --- Holes ---
	UFUNCTION(BlueprintCallable, Category = "Course")
	TArray<UHoleData*> GetAllHoleData() const;

	UFUNCTION(BlueprintPure, Category = "Course")
	UHoleData* GetHoleData(int32 holeNumber) const;

	UFUNCTION(BlueprintCallable, Category = "Course")
	bool GetHoleState(int32 holeNumber, FHoleState& outState) const;

	UFUNCTION(BlueprintPure, Category = "Course")
	bool IsHoleUnlocked(int32 holeNumber) const;

	UFUNCTION(BlueprintPure, Category = "Course")
	bool CanUnlockHole(int32 holeNumber) const;

	UFUNCTION(BlueprintCallable, Category = "Course")
	bool TryUnlockHole(int32 holeNumber);

	UFUNCTION(BlueprintPure, Category = "Course")
	int32 GetUnlockedHoleCount() const;

	// Unlocked holes whose HoleData has countsForRevenue (used by the daily revenue calc).
	UFUNCTION(BlueprintPure, Category = "Golf|Course")
	int32 GetRevenueHoleCount() const;

	UFUNCTION(BlueprintPure, Category = "Course")
	TArray<int32> GetUnlockedHoleNumbers() const;

	// --- Conditions ---
	UFUNCTION(BlueprintPure, Category = "Course")
	float GetCondition(int32 holeNumber, EConditionStat stat) const;

	// Adds delta to the stat, clamped to 0.0 - 1.0.
	UFUNCTION(BlueprintCallable, Category = "Course")
	void ApplyConditionChange(int32 holeNumber, EConditionStat stat, float delta);

private:
	FHoleState* FindHoleState(int32 holeNumber);
	const FHoleState* FindHoleState(int32 holeNumber) const;

	UPROPERTY()
	TArray<TObjectPtr<UHoleData>> holeDataList;

	UPROPERTY()
	TArray<FHoleState> holeStates;

	UPROPERTY()
	int32 playerCash = startingCash;

	UPROPERTY()
	int32 ownedEquipmentTier = 0;
};
