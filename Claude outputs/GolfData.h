// GolfData.h
// Read-only definitions (Data Assets). Port of the Godot .tres resources.
// Create instances in the Content Browser: Add > Miscellaneous > Data Asset > pick the class.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/GolfTypes.h"
#include "GolfData.generated.h"

UCLASS(BlueprintType)
class GOLFGAME_API UHoleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hole")
	int32 holeNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hole")
	FText displayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hole")
	int32 par = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	int32 requiredEquipmentTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	int32 unlockCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	bool startsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float startingFairwayCondition = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float startingRoughCondition = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float startingGreenCondition = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazards")
	bool hasWaterHazard = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazards")
	bool hasSandHazard = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cosmetic")
	int32 hazardColorTheme = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cosmetic")
	int32 flagColorTheme = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cosmetic")
	int32 flowerBedTheme = 0;

	// False for non-golf areas (hub/clubhouse levels): no revenue, hidden from the unlock list.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hole")
	bool countsForRevenue = true;

	// Level to open for this area (replaces holeScene).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hole")
	TSoftObjectPtr<UWorld> level;

	UFUNCTION(BlueprintPure, Category = "Hole")
	bool IsUnlockableWith(int32 currentTier, int32 balance) const
	{
		return currentTier >= requiredEquipmentTier && balance >= unlockCost;
	}
};

UCLASS(BlueprintType)
class GOLFGAME_API UEquipmentData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FText equipmentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	int32 tier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	int32 cost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (MultiLine = "true"))
	FText description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	EConditionStat affectsStat = EConditionStat::Fairway;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	float restoreAmount = 0.0f;

	// Mower stats for this tier (read by the mower Blueprint when the tier changes).
	// Cut radius in cm: half of the mowed strip width.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mower", meta = (ClampMin = "10.0"))
	float cutRadiusCm = 60.0f;

	// Multiplier on the player's walk speed while the mower is active (1.0 = normal).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mower", meta = (ClampMin = "0.1"))
	float moveSpeedMultiplier = 1.0f;
};

UCLASS(BlueprintType)
class GOLFGAME_API UStaffData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	FText staffName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	EStaffRole role = EStaffRole::Groundskeeper;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float skillLevel = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	int32 dailyWage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Staff")
	int32 hireCost = 0;
};

UCLASS(BlueprintType)
class GOLFGAME_API UApplicationPairData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Application")
	FName pairId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Application")
	TObjectPtr<UStaffData> applicantA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Application")
	TObjectPtr<UStaffData> applicantB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Application", meta = (MultiLine = "true"))
	FText flavorText;
};

UCLASS(BlueprintType)
class GOLFGAME_API URoomData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	FName roomID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	FText displayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	int32 tierRequired = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	int32 buildCost = 0;
};

UCLASS(BlueprintType)
class GOLFGAME_API UFurnitureItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
	FName itemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
	FText displayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
	int32 cost = 0;

	// Room IDs this item may be placed in.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
	TArray<FName> roomTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Furniture")
	FIntPoint footprintSize = FIntPoint(1, 1);
};

// One asset that lists every definition, so the subsystems never hardcode filenames
// (replaces the hole00..hole09 path loop). Assign it in Project Settings > Game > Golf Game.
UCLASS(BlueprintType)
class GOLFGAME_API UGolfDataRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TObjectPtr<UHoleData>> holes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TObjectPtr<UEquipmentData>> equipment;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TObjectPtr<URoomData>> rooms;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TObjectPtr<UFurnitureItemData>> furniture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TObjectPtr<UApplicationPairData>> applicationPairs;
};
