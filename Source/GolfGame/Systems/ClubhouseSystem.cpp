// ClubhouseSystem.cpp

#include "Systems/ClubhouseSystem.h"
#include "Systems/CourseManager.h"
#include "Data/GolfData.h"
#include "Data/GolfSettings.h"
#include "Engine/GameInstance.h"

namespace
{
	bool RectsOverlap(FIntPoint aPos, FIntPoint aSize, FIntPoint bPos, FIntPoint bSize)
	{
		return aPos.X < bPos.X + bSize.X
			&& aPos.X + aSize.X > bPos.X
			&& aPos.Y < bPos.Y + bSize.Y
			&& aPos.Y + aSize.Y > bPos.Y;
	}

	FIntPoint FootprintOf(const UFurnitureItemData* item)
	{
		return FIntPoint(FMath::Max(1, item->footprintSize.X), FMath::Max(1, item->footprintSize.Y));
	}
}

void UClubhouseSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UCourseManager>();
	Super::Initialize(Collection);

	clubhouseTier = 0;
	builtRooms.Reset();
	placedItems.Reset();
	allRooms.Reset();

	const UGolfSettings* settings = GetDefault<UGolfSettings>();
	UGolfDataRegistry* registry = settings ? settings->dataRegistry.LoadSynchronous() : nullptr;
	if (registry)
	{
		for (URoomData* room : registry->rooms)
		{
			if (room)
			{
				allRooms.Add(room);
			}
		}
	}
}

URoomData* UClubhouseSystem::GetRoomData(FName roomID) const
{
	for (const TObjectPtr<URoomData>& room : allRooms)
	{
		if (room && room->roomID == roomID)
		{
			return room;
		}
	}
	return nullptr;
}

bool UClubhouseSystem::CanUnlock(FName roomID) const
{
	const URoomData* room = GetRoomData(roomID);
	// Tier gate only, matching Godot. BuildRoom checks "already built" separately.
	return room && clubhouseTier >= room->tierRequired;
}

bool UClubhouseSystem::BuildRoom(FName roomID)
{
	if (!CanUnlock(roomID) || builtRooms.Contains(roomID))
	{
		return false;
	}

	UGameInstance* gameInstance = GetGameInstance();
	UCourseManager* course = gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
	const URoomData* room = GetRoomData(roomID);
	if (!course || !room || !course->TrySpendCash(room->buildCost))
	{
		return false;
	}

	builtRooms.Add(roomID);
	onRoomBuilt.Broadcast(roomID);
	return true;
}

bool UClubhouseSystem::UpgradeClubhouseTier(int32 cost)
{
	UGameInstance* gameInstance = GetGameInstance();
	UCourseManager* course = gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
	if (!course || !course->TrySpendCash(cost))
	{
		return false;
	}

	++clubhouseTier;
	return true;
}

bool UClubhouseSystem::PlaceItem(UFurnitureItemData* item, FName roomID, FIntPoint gridPosition)
{
	if (!item || !builtRooms.Contains(roomID) || !item->roomTypes.Contains(roomID))
	{
		return false;
	}

	const FIntPoint newSize = FootprintOf(item);

	// Conflict check first. Only after the whole loop passes do we charge and add.
	for (const FPlacedItem& existing : placedItems)
	{
		if (existing.roomID != roomID || !existing.itemData)
		{
			continue;
		}
		if (RectsOverlap(gridPosition, newSize, existing.gridPosition, FootprintOf(existing.itemData)))
		{
			return false;
		}
	}

	UGameInstance* gameInstance = GetGameInstance();
	UCourseManager* course = gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
	if (!course || !course->TrySpendCash(item->cost))
	{
		return false;
	}

	FPlacedItem placed;
	placed.itemData = item;
	placed.roomID = roomID;
	placed.gridPosition = gridPosition;
	placedItems.Add(placed);
	onItemPlaced.Broadcast(placed);
	return true;
}

bool UClubhouseSystem::RemoveItem(FName roomID, FIntPoint gridPosition)
{
	for (int32 i = 0; i < placedItems.Num(); ++i)
	{
		const FPlacedItem& existing = placedItems[i];
		if (existing.roomID != roomID || !existing.itemData)
		{
			continue;
		}
		// A cell belongs to an item if it falls inside that item's footprint.
		if (RectsOverlap(gridPosition, FIntPoint(1, 1), existing.gridPosition, FootprintOf(existing.itemData)))
		{
			const FPlacedItem removed = existing;
			placedItems.RemoveAt(i);
			onItemRemoved.Broadcast(removed);
			return true;
		}
	}
	return false;
}

TArray<FPlacedItem> UClubhouseSystem::GetItemsInRoom(FName roomID) const
{
	TArray<FPlacedItem> result;
	for (const FPlacedItem& placed : placedItems)
	{
		if (placed.roomID == roomID)
		{
			result.Add(placed);
		}
	}
	return result;
}
