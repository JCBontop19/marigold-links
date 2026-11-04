// StaffSystem.cpp

#include "Systems/StaffSystem.h"
#include "Systems/CourseManager.h"
#include "Data/GolfData.h"
#include "Engine/GameInstance.h"

void UStaffSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// CourseManager must exist first: hiring spends its cash.
	Collection.InitializeDependency<UCourseManager>();
	Super::Initialize(Collection);

	roster.Reset();
	rejectedApplicants.Reset();
}

UCourseManager* UStaffSystem::GetCourse() const
{
	UGameInstance* gameInstance = GetGameInstance();
	return gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
}

FStaffState* UStaffSystem::FindStaffState(const UStaffData* staff)
{
	return roster.FindByPredicate([staff](const FStaffState& s) { return s.staffData == staff; });
}

const FStaffState* UStaffSystem::FindStaffState(const UStaffData* staff) const
{
	return roster.FindByPredicate([staff](const FStaffState& s) { return s.staffData == staff; });
}

bool UStaffSystem::IsHired(const UStaffData* staff) const
{
	return FindStaffState(staff) != nullptr;
}

bool UStaffSystem::HireStaff(UStaffData* staff)
{
	UCourseManager* course = GetCourse();
	if (!staff || !course || IsHired(staff))
	{
		return false;
	}
	if (!course->TrySpendCash(staff->hireCost))
	{
		return false;
	}

	FStaffState newState;
	newState.staffData = staff;
	roster.Add(newState);
	onRosterChanged.Broadcast();
	return true;
}

bool UStaffSystem::FireStaff(UStaffData* staff)
{
	const int32 removed = roster.RemoveAll([staff](const FStaffState& s) { return s.staffData == staff; });
	if (removed > 0)
	{
		onRosterChanged.Broadcast();
		return true;
	}
	return false;
}

bool UStaffSystem::AssignToHole(UStaffData* staff, int32 holeNumber)
{
	FStaffState* state = FindStaffState(staff);
	if (!state)
	{
		return false;
	}

	if (holeNumber != -1)
	{
		UCourseManager* course = GetCourse();
		if (!course || !course->IsHoleUnlocked(holeNumber))
		{
			return false;
		}
	}

	state->assignedHoleNumber = holeNumber;
	onRosterChanged.Broadcast();
	return true;
}

void UStaffSystem::RejectApplicant(UStaffData* staff)
{
	if (staff)
	{
		rejectedApplicants.Add(staff);
	}
}

int32 UStaffSystem::GetTotalWages() const
{
	int32 total = 0;
	for (const FStaffState& state : roster)
	{
		if (state.staffData)
		{
			total += state.staffData->dailyWage;
		}
	}
	return total;
}

void UStaffSystem::ApplyMoralePenalty(float amount)
{
	for (FStaffState& state : roster)
	{
		state.morale = FMath::Clamp(state.morale - amount, 0.0f, 1.0f);
	}
	onRosterChanged.Broadcast();
}

float UStaffSystem::GetMaintenanceForHole(int32 holeNumber) const
{
	float total = 0.0f;
	for (const FStaffState& state : roster)
	{
		if (state.assignedHoleNumber == holeNumber
			&& state.staffData
			&& state.staffData->role == EStaffRole::Groundskeeper)
		{
			total += baseRestore * (1.0f + state.staffData->skillLevel);
		}
	}
	return total;
}

TMap<int32, float> UStaffSystem::GetMaintenanceEffects() const
{
	TMap<int32, float> effects;
	for (const FStaffState& state : roster)
	{
		if (state.assignedHoleNumber >= 0
			&& state.staffData
			&& state.staffData->role == EStaffRole::Groundskeeper)
		{
			effects.FindOrAdd(state.assignedHoleNumber) += baseRestore * (1.0f + state.staffData->skillLevel);
		}
	}
	return effects;
}
