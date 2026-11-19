// GolfSaveGame.h
// Everything that gets written to a save slot. Plain data only: data assets are stored as soft
// references, and the systems rebuild their runtime state from this in ReadFromSave.
//
// NOT saved: the cut-mask render target (64 MB, cleared every day). A load keeps whatever is on
// the mask in the current session, and the autosave at the start of each day happens when it is empty.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/GolfTypes.h"
#include "GolfSaveGame.generated.h"

class UStaffData;
class UFurnitureItemData;

USTRUCT()
struct GOLFGAME_API FSavedStaff
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UStaffData> staffData;

	UPROPERTY()
	int32 assignedHoleNumber = -1;

	UPROPERTY()
	float morale = 1.0f;
};

USTRUCT()
struct GOLFGAME_API FSavedPlacedItem
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UFurnitureItemData> itemData;

	UPROPERTY()
	FName roomID;

	UPROPERTY()
	FIntPoint gridPosition = FIntPoint::ZeroValue;
};

UCLASS()
class GOLFGAME_API UGolfSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Bump when the layout changes. Loading a save with a higher version is refused.
	static constexpr int32 currentVersion = 1;

	UPROPERTY()
	int32 version = currentVersion;

	UPROPERTY()
	FDateTime savedAt;

	// --- CourseManager ---
	UPROPERTY()
	int32 playerCash = 0;

	UPROPERTY()
	int32 ownedEquipmentTier = 0;

	UPROPERTY()
	TArray<FHoleState> holeStates;

	// --- DayCycle ---
	UPROPERTY()
	int32 currentDay = 1;

	UPROPERTY()
	float currentTimeMinutes = 300.0f;

	UPROPERTY()
	float closeElapsedMinutes = 0.0f;

	UPROPERTY()
	EDayPhase currentPhase = EDayPhase::MorningPrep;

	UPROPERTY()
	FDayReport lastReport;

	// --- StaffSystem ---
	UPROPERTY()
	TArray<FSavedStaff> roster;

	UPROPERTY()
	TArray<TSoftObjectPtr<UStaffData>> rejectedApplicants;

	// --- GuestSystem ---
	UPROPERTY()
	float reputation = 0.5f;

	UPROPERTY()
	FGuestDayResult lastGuestResult;

	// --- HazardSystem ---
	UPROPERTY()
	TArray<FHazardState> hazardStates;

	// --- ClubhouseSystem ---
	UPROPERTY()
	int32 clubhouseTier = 0;

	UPROPERTY()
	TArray<FName> builtRooms;

	UPROPERTY()
	TArray<FSavedPlacedItem> placedItems;
};
