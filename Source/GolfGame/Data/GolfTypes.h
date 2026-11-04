// GolfTypes.h
// Shared enums and runtime-state structs. Definitions (Data Assets) are read-only;
// anything that changes during play lives in the structs below, owned by the subsystems.

#pragma once

#include "CoreMinimal.h"
#include "GolfTypes.generated.h"

class UStaffData;
class UFurnitureItemData;

UENUM(BlueprintType)
enum class EStaffRole : uint8
{
	Groundskeeper,
	ProShop,
	Instructor
};

UENUM(BlueprintType)
enum class EConditionStat : uint8
{
	Fairway,
	Rough,
	Green
};

UENUM(BlueprintType)
enum class EDayPhase : uint8
{
	MorningPrep,
	OpenHours,
	Close
};

// Runtime state for one hole (the HoleData asset holds the fixed definition).
USTRUCT(BlueprintType)
struct GOLFGAME_API FHoleState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hole")
	int32 holeNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hole")
	bool isUnlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hole")
	float fairwayCondition = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hole")
	float roughCondition = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hole")
	float greenCondition = 1.0f;
};

// Runtime state for one hired staff member (the StaffData asset holds the fixed definition).
USTRUCT(BlueprintType)
struct GOLFGAME_API FStaffState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Staff")
	TObjectPtr<UStaffData> staffData = nullptr;

	// -1 means unassigned.
	UPROPERTY(BlueprintReadOnly, Category = "Staff")
	int32 assignedHoleNumber = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Staff")
	float morale = 1.0f;
};

USTRUCT(BlueprintType)
struct GOLFGAME_API FDayReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 day = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 revenue = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 wages = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 net = 0;
};

USTRUCT(BlueprintType)
struct GOLFGAME_API FPlacedItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Clubhouse")
	TObjectPtr<UFurnitureItemData> itemData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Clubhouse")
	FName roomID;

	UPROPERTY(BlueprintReadOnly, Category = "Clubhouse")
	FIntPoint gridPosition = FIntPoint::ZeroValue;
};
