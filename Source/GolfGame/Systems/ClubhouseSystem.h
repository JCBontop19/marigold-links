// ClubhouseSystem.h
// Port of the ClubhouseSystem autoload: clubhouse tier, buildable rooms, furniture placement.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GolfTypes.h"
#include "ClubhouseSystem.generated.h"

class URoomData;
class UFurnitureItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomBuilt, FName, roomID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPlaced, FPlacedItem, item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, FPlacedItem, item);

UCLASS()
class GOLFGAME_API UClubhouseSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Clubhouse")
	FOnRoomBuilt onRoomBuilt;

	UPROPERTY(BlueprintAssignable, Category = "Clubhouse")
	FOnItemPlaced onItemPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Clubhouse")
	FOnItemRemoved onItemRemoved;

	// Loaded from the data registry. Public so tests can substitute their own rooms.
	UPROPERTY(BlueprintReadOnly, Category = "Clubhouse")
	TArray<TObjectPtr<URoomData>> allRooms;

	UFUNCTION(BlueprintPure, Category = "Clubhouse")
	int32 GetClubhouseTier() const { return clubhouseTier; }

	UFUNCTION(BlueprintPure, Category = "Clubhouse")
	URoomData* GetRoomData(FName roomID) const;

	UFUNCTION(BlueprintPure, Category = "Clubhouse")
	bool IsRoomBuilt(FName roomID) const { return builtRooms.Contains(roomID); }

	// Tier requirement met and not already built. (Cash is checked when building.)
	UFUNCTION(BlueprintPure, Category = "Clubhouse")
	bool CanUnlock(FName roomID) const;

	UFUNCTION(BlueprintCallable, Category = "Clubhouse")
	bool BuildRoom(FName roomID);

	UFUNCTION(BlueprintCallable, Category = "Clubhouse")
	bool UpgradeClubhouseTier(int32 cost);

	// Fails if: room not built, item not allowed in this room, footprint overlaps an
	// existing item, or the player cannot afford it.
	UFUNCTION(BlueprintCallable, Category = "Clubhouse")
	bool PlaceItem(UFurnitureItemData* item, FName roomID, FIntPoint gridPosition);

	// Removes the item occupying that grid cell. No refund.
	UFUNCTION(BlueprintCallable, Category = "Clubhouse")
	bool RemoveItem(FName roomID, FIntPoint gridPosition);

	UFUNCTION(BlueprintCallable, Category = "Clubhouse")
	TArray<FPlacedItem> GetItemsInRoom(FName roomID) const;

private:
	UPROPERTY()
	int32 clubhouseTier = 0;

	UPROPERTY()
	TSet<FName> builtRooms;

	UPROPERTY()
	TArray<FPlacedItem> placedItems;
};
