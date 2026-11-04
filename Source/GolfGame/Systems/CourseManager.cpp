// CourseManager.cpp

#include "Systems/CourseManager.h"
#include "Data/GolfData.h"
#include "Data/GolfSettings.h"

void UCourseManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	holeDataList.Reset();
	holeStates.Reset();
	playerCash = startingCash;
	ownedEquipmentTier = 0;

	const UGolfSettings* settings = GetDefault<UGolfSettings>();
	UGolfDataRegistry* registry = settings ? settings->dataRegistry.LoadSynchronous() : nullptr;
	if (!registry)
	{
		UE_LOG(LogTemp, Error, TEXT("CourseManager: no Golf Data Registry assigned. Project Settings > Game > Golf Game."));
		return;
	}

	for (UHoleData* hole : registry->holes)
	{
		if (hole)
		{
			holeDataList.Add(hole);
		}
	}

	holeDataList.Sort([](const UHoleData& a, const UHoleData& b)
	{
		return a.holeNumber < b.holeNumber;
	});

	for (const UHoleData* hole : holeDataList)
	{
		FHoleState state;
		state.holeNumber = hole->holeNumber;
		state.isUnlocked = hole->startsUnlocked;
		state.fairwayCondition = hole->startingFairwayCondition;
		state.roughCondition = hole->startingRoughCondition;
		state.greenCondition = hole->startingGreenCondition;
		holeStates.Add(state);
	}

	UE_LOG(LogTemp, Log, TEXT("CourseManager: loaded %d holes"), holeDataList.Num());
}

// ---------------------------------------------------------------- Cash

void UCourseManager::AddCash(int32 amount)
{
	playerCash += amount;
	onCashChanged.Broadcast(playerCash);
}

bool UCourseManager::TrySpendCash(int32 amount)
{
	if (amount < 0 || playerCash < amount)
	{
		return false;
	}
	playerCash -= amount;
	onCashChanged.Broadcast(playerCash);
	return true;
}

// ---------------------------------------------------------------- Equipment

bool UCourseManager::PurchaseEquipment(UEquipmentData* equipment)
{
	if (!equipment || equipment->tier <= ownedEquipmentTier)
	{
		return false;
	}
	if (!TrySpendCash(equipment->cost))
	{
		return false;
	}
	ownedEquipmentTier = equipment->tier;
	onEquipmentTierChanged.Broadcast(ownedEquipmentTier);
	return true;
}

// ---------------------------------------------------------------- Holes

TArray<UHoleData*> UCourseManager::GetAllHoleData() const
{
	TArray<UHoleData*> result;
	result.Reserve(holeDataList.Num());
	for (const TObjectPtr<UHoleData>& hole : holeDataList)
	{
		result.Add(hole);
	}
	return result;
}

UHoleData* UCourseManager::GetHoleData(int32 holeNumber) const
{
	for (const TObjectPtr<UHoleData>& hole : holeDataList)
	{
		if (hole && hole->holeNumber == holeNumber)
		{
			return hole;
		}
	}
	return nullptr;
}

FHoleState* UCourseManager::FindHoleState(int32 holeNumber)
{
	return holeStates.FindByPredicate([holeNumber](const FHoleState& s) { return s.holeNumber == holeNumber; });
}

const FHoleState* UCourseManager::FindHoleState(int32 holeNumber) const
{
	return holeStates.FindByPredicate([holeNumber](const FHoleState& s) { return s.holeNumber == holeNumber; });
}

bool UCourseManager::GetHoleState(int32 holeNumber, FHoleState& outState) const
{
	if (const FHoleState* state = FindHoleState(holeNumber))
	{
		outState = *state;
		return true;
	}
	return false;
}

bool UCourseManager::IsHoleUnlocked(int32 holeNumber) const
{
	const FHoleState* state = FindHoleState(holeNumber);
	return state && state->isUnlocked;
}

bool UCourseManager::CanUnlockHole(int32 holeNumber) const
{
	const UHoleData* data = GetHoleData(holeNumber);
	const FHoleState* state = FindHoleState(holeNumber);
	if (!data || !state || state->isUnlocked)
	{
		return false;
	}
	return data->IsUnlockableWith(ownedEquipmentTier, playerCash);
}

bool UCourseManager::TryUnlockHole(int32 holeNumber)
{
	if (!CanUnlockHole(holeNumber))
	{
		return false;
	}

	const UHoleData* data = GetHoleData(holeNumber);
	FHoleState* state = FindHoleState(holeNumber);

	TrySpendCash(data->unlockCost);
	state->isUnlocked = true;
	onHoleUnlocked.Broadcast(holeNumber);
	return true;
}

int32 UCourseManager::GetUnlockedHoleCount() const
{
	int32 count = 0;
	for (const FHoleState& state : holeStates)
	{
		if (state.isUnlocked)
		{
			++count;
		}
	}
	return count;
}

int32 UCourseManager::GetRevenueHoleCount() const
{
	int32 count = 0;
	for (const FHoleState& state : holeStates)
	{
		const UHoleData* hole = GetHoleData(state.holeNumber);
		if (state.isUnlocked && hole && hole->countsForRevenue)
		{
			++count;
		}
	}
	return count;
}

TArray<int32> UCourseManager::GetUnlockedHoleNumbers() const
{
	TArray<int32> result;
	for (const FHoleState& state : holeStates)
	{
		if (state.isUnlocked)
		{
			result.Add(state.holeNumber);
		}
	}
	return result;
}

// ---------------------------------------------------------------- Conditions

float UCourseManager::GetCondition(int32 holeNumber, EConditionStat stat) const
{
	const FHoleState* state = FindHoleState(holeNumber);
	if (!state)
	{
		return 0.0f;
	}
	switch (stat)
	{
	case EConditionStat::Fairway: return state->fairwayCondition;
	case EConditionStat::Rough:   return state->roughCondition;
	case EConditionStat::Green:   return state->greenCondition;
	}
	return 0.0f;
}

void UCourseManager::ApplyConditionChange(int32 holeNumber, EConditionStat stat, float delta)
{
	FHoleState* state = FindHoleState(holeNumber);
	if (!state)
	{
		return;
	}

	float* value = nullptr;
	switch (stat)
	{
	case EConditionStat::Fairway: value = &state->fairwayCondition; break;
	case EConditionStat::Rough:   value = &state->roughCondition;   break;
	case EConditionStat::Green:   value = &state->greenCondition;   break;
	}
	if (!value)
	{
		return;
	}

	*value = FMath::Clamp(*value + delta, 0.0f, 1.0f);
	onHoleConditionChanged.Broadcast(holeNumber, stat);
}
