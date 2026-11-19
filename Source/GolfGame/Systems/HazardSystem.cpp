// HazardSystem.cpp

#include "Systems/HazardSystem.h"
#include "Systems/CourseManager.h"
#include "Data/GolfData.h"
#include "Data/GolfSaveGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

void UHazardSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UCourseManager>();
	Super::Initialize(Collection);

	states.Reset();
	if (const UCourseManager* course = GetCourse())
	{
		for (const UHoleData* hole : course->GetAllHoleData())
		{
			if (hole && (hole->hasSandHazard || hole->hasWaterHazard))
			{
				FHazardState state;
				state.holeNumber = hole->holeNumber;
				states.Add(state);
			}
		}
	}
}

UCourseManager* UHazardSystem::GetCourse() const
{
	UGameInstance* gameInstance = GetGameInstance();
	return gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
}

FHazardState* UHazardSystem::FindState(int32 holeNumber)
{
	return states.FindByPredicate([holeNumber](const FHazardState& s) { return s.holeNumber == holeNumber; });
}

const FHazardState* UHazardSystem::FindState(int32 holeNumber) const
{
	return states.FindByPredicate([holeNumber](const FHazardState& s) { return s.holeNumber == holeNumber; });
}

bool UHazardSystem::HasSand(int32 holeNumber) const
{
	const UCourseManager* course = GetCourse();
	const UHoleData* hole = course ? course->GetHoleData(holeNumber) : nullptr;
	return hole && hole->hasSandHazard;
}

bool UHazardSystem::HasWater(int32 holeNumber) const
{
	const UCourseManager* course = GetCourse();
	const UHoleData* hole = course ? course->GetHoleData(holeNumber) : nullptr;
	return hole && hole->hasWaterHazard;
}

bool UHazardSystem::NeedsRaking(int32 holeNumber) const
{
	const FHazardState* state = FindState(holeNumber);
	return state && HasSand(holeNumber) && state->sandNeedsRaking;
}

float UHazardSystem::GetWaterCondition(int32 holeNumber) const
{
	const FHazardState* state = FindState(holeNumber);
	return (state && HasWater(holeNumber)) ? state->waterCondition : 1.0f;
}

float UHazardSystem::GetSatisfactionPenalty(int32 holeNumber) const
{
	float penalty = 0.0f;
	if (NeedsRaking(holeNumber))
	{
		penalty += sandPenalty;
	}
	if (HasWater(holeNumber))
	{
		penalty += (1.0f - FMath::Clamp(GetWaterCondition(holeNumber), 0.0f, 1.0f)) * waterPenaltyMax;
	}
	return penalty;
}

int32 UHazardSystem::GetAttentionCount() const
{
	const UCourseManager* course = GetCourse();
	if (!course)
	{
		return 0;
	}

	int32 count = 0;
	for (const int32 holeNumber : course->GetUnlockedHoleNumbers())
	{
		if (NeedsRaking(holeNumber))
		{
			++count;
		}
		if (HasWater(holeNumber) && GetWaterCondition(holeNumber) < waterAttentionThreshold)
		{
			++count;
		}
	}
	return count;
}

bool UHazardSystem::RakeSand(int32 holeNumber)
{
	FHazardState* state = FindState(holeNumber);
	if (!state || !HasSand(holeNumber) || !state->sandNeedsRaking)
	{
		return false;
	}
	state->sandNeedsRaking = false;
	UE_LOG(LogTemp, Display, TEXT("[Hazards] hole %d bunker raked"), holeNumber);
	onHazardChanged.Broadcast(holeNumber);
	return true;
}

bool UHazardSystem::CleanWater(int32 holeNumber)
{
	FHazardState* state = FindState(holeNumber);
	if (!state || !HasWater(holeNumber) || state->waterCondition >= 0.999f)
	{
		return false;
	}
	state->waterCondition = 1.0f;
	UE_LOG(LogTemp, Display, TEXT("[Hazards] hole %d water cleaned"), holeNumber);
	onHazardChanged.Broadcast(holeNumber);
	return true;
}

void UHazardSystem::StartNewDay()
{
	const UCourseManager* course = GetCourse();
	if (!course)
	{
		return;
	}

	// Only holes the player can actually play wear down.
	const TSet<int32> unlocked(course->GetUnlockedHoleNumbers());

	for (FHazardState& state : states)
	{
		if (!unlocked.Contains(state.holeNumber))
		{
			continue;
		}

		bool changed = false;

		if (HasSand(state.holeNumber) && !state.sandNeedsRaking && FMath::FRand() < sandMessChance)
		{
			state.sandNeedsRaking = true;
			changed = true;
		}

		if (HasWater(state.holeNumber) && state.waterCondition > 0.0f)
		{
			state.waterCondition = FMath::Max(0.0f, state.waterCondition - waterDecayPerDay);
			changed = true;
		}

		if (changed)
		{
			onHazardChanged.Broadcast(state.holeNumber);
		}
	}

	LogStatus();
}

void UHazardSystem::DebugMessAll()
{
	for (FHazardState& state : states)
	{
		state.sandNeedsRaking = HasSand(state.holeNumber);
		if (HasWater(state.holeNumber))
		{
			state.waterCondition = 0.3f;
		}
		onHazardChanged.Broadcast(state.holeNumber);
	}
	LogStatus();
}

void UHazardSystem::DebugSetSandNeedsRaking(int32 holeNumber, bool needsRaking)
{
	if (FHazardState* state = FindState(holeNumber))
	{
		state->sandNeedsRaking = needsRaking;
		onHazardChanged.Broadcast(holeNumber);
	}
}

void UHazardSystem::DebugSetWater(int32 holeNumber, float waterCondition)
{
	if (FHazardState* state = FindState(holeNumber))
	{
		state->waterCondition = FMath::Clamp(waterCondition, 0.0f, 1.0f);
		onHazardChanged.Broadcast(holeNumber);
	}
}

void UHazardSystem::LogStatus() const
{
	for (const FHazardState& state : states)
	{
		UE_LOG(LogTemp, Display, TEXT("[Hazards] hole %d: sand %s, water %.2f, penalty %.3f"),
			state.holeNumber,
			HasSand(state.holeNumber) ? (state.sandNeedsRaking ? TEXT("NEEDS RAKING") : TEXT("ok")) : TEXT("none"),
			GetWaterCondition(state.holeNumber),
			GetSatisfactionPenalty(state.holeNumber));
	}
}

// ---------------------------------------------------------------- Save and load

void UHazardSystem::WriteToSave(UGolfSaveGame& save) const
{
	save.hazardStates = states;
}

void UHazardSystem::ReadFromSave(const UGolfSaveGame& save)
{
	for (FHazardState& state : states)
	{
		state.sandNeedsRaking = false;
		state.waterCondition = 1.0f;
	}

	for (const FHazardState& saved : save.hazardStates)
	{
		if (FHazardState* state = FindState(saved.holeNumber))
		{
			state->sandNeedsRaking = saved.sandNeedsRaking;
			state->waterCondition = FMath::Clamp(saved.waterCondition, 0.0f, 1.0f);
		}
	}

	for (const FHazardState& state : states)
	{
		onHazardChanged.Broadcast(state.holeNumber);
	}
}

// ---------------------------------------------------------------- Console

static UHazardSystem* GetHazardsFromWorld(UWorld* world)
{
	UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;
	return gameInstance ? gameInstance->GetSubsystem<UHazardSystem>() : nullptr;
}

static FAutoConsoleCommandWithWorldAndArgs GolfMessHazardsCmd(
	TEXT("golf.MessHazards"),
	TEXT("golf.MessHazards: every hazard hole gets an unraked bunker and dirty water (water 0.3)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& args, UWorld* world)
	{
		if (UHazardSystem* hazards = GetHazardsFromWorld(world))
		{
			hazards->DebugMessAll();
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs GolfHazardStatusCmd(
	TEXT("golf.HazardStatus"),
	TEXT("golf.HazardStatus: print every hazard hole's state to the Output Log."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& args, UWorld* world)
	{
		if (UHazardSystem* hazards = GetHazardsFromWorld(world))
		{
			hazards->LogStatus();
		}
	}));
