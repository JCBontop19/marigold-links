// GuestSystem.cpp

#include "Systems/GuestSystem.h"
#include "Systems/CourseManager.h"
#include "Systems/HazardSystem.h"
#include "Data/GolfData.h"
#include "Data/GolfSaveGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

void UGuestSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UCourseManager>();
	Collection.InitializeDependency<UHazardSystem>();
	Super::Initialize(Collection);

	reputation = startingReputation;
	lastResult = FGuestDayResult();
}

UCourseManager* UGuestSystem::GetCourse() const
{
	UGameInstance* gameInstance = GetGameInstance();
	return gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
}

float UGuestSystem::GetArrivalMultiplier() const
{
	return FMath::Lerp(minArrivalMultiplier, maxArrivalMultiplier, FMath::Clamp(reputation, 0.0f, 1.0f));
}

float UGuestSystem::GetHoleSatisfaction(int32 holeNumber) const
{
	const UCourseManager* course = GetCourse();
	if (!course || !course->GetHoleData(holeNumber))
	{
		return 0.0f;
	}

	float score =
		course->GetCondition(holeNumber, EConditionStat::Fairway) * fairwayWeight +
		course->GetCondition(holeNumber, EConditionStat::Rough) * roughWeight +
		course->GetCondition(holeNumber, EConditionStat::Green) * greenWeight;

	// An unraked bunker or a dirty pond takes a bite out of it.
	UGameInstance* gameInstance = GetGameInstance();
	if (const UHazardSystem* hazards = gameInstance ? gameInstance->GetSubsystem<UHazardSystem>() : nullptr)
	{
		score -= hazards->GetSatisfactionPenalty(holeNumber);
	}
	return FMath::Clamp(score, 0.0f, 1.0f);
}

int32 UGuestSystem::GuestsForSatisfaction(float satisfaction) const
{
	const float arrivals = baseGuestsPerHole * GetArrivalMultiplier();
	const float playFraction = FMath::Lerp(minPlayFraction, 1.0f, FMath::Clamp(satisfaction, 0.0f, 1.0f));
	return FMath::RoundToInt(arrivals * playFraction);
}

int32 UGuestSystem::GetExpectedGuests(int32 holeNumber) const
{
	const UCourseManager* course = GetCourse();
	const UHoleData* hole = course ? course->GetHoleData(holeNumber) : nullptr;
	if (!hole || !hole->countsForRevenue)
	{
		return 0;
	}
	return GuestsForSatisfaction(GetHoleSatisfaction(holeNumber));
}

FGuestDayResult UGuestSystem::ProcessDay()
{
	FGuestDayResult result;
	result.reputation = reputation;

	UCourseManager* course = GetCourse();
	if (!course)
	{
		lastResult = result;
		return result;
	}

	float satisfactionSum = 0.0f;
	int32 holeCount = 0;
	for (const int32 holeNumber : course->GetUnlockedHoleNumbers())
	{
		const UHoleData* hole = course->GetHoleData(holeNumber);
		if (!hole || !hole->countsForRevenue)
		{
			continue;
		}

		const float satisfaction = GetHoleSatisfaction(holeNumber);
		result.guests += GuestsForSatisfaction(satisfaction);
		satisfactionSum += satisfaction;
		++holeCount;
	}

	result.revenue = result.guests * greenFee;

	// Reputation moves only if there was a course to judge, and only after today's arrivals
	// were counted with yesterday's reputation.
	if (holeCount > 0)
	{
		result.satisfaction = satisfactionSum / holeCount;
		reputation = FMath::Clamp(reputation + (result.satisfaction - reputation) * reputationRate, 0.0f, 1.0f);
	}
	result.reputation = reputation;

	UE_LOG(LogTemp, Display, TEXT("[Guests] %d guests, satisfaction %.2f, revenue %d, reputation now %.3f"),
		result.guests, result.satisfaction, result.revenue, result.reputation);

	lastResult = result;
	return result;
}

void UGuestSystem::DebugSetReputation(float newReputation)
{
	reputation = FMath::Clamp(newReputation, 0.0f, 1.0f);
	UE_LOG(LogTemp, Display, TEXT("[Guests] reputation set to %.3f"), reputation);
}

// ---------------------------------------------------------------- Save and load

void UGuestSystem::WriteToSave(UGolfSaveGame& save) const
{
	save.reputation = reputation;
	save.lastGuestResult = lastResult;
}

void UGuestSystem::ReadFromSave(const UGolfSaveGame& save)
{
	reputation = FMath::Clamp(save.reputation, 0.0f, 1.0f);
	lastResult = save.lastGuestResult;
}

// ---------------------------------------------------------------- Console

static FAutoConsoleCommandWithWorldAndArgs GolfSetReputationCmd(
	TEXT("golf.SetReputation"),
	TEXT("golf.SetReputation <0..1>: set the course reputation (guest arrivals scale with it)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& args, UWorld* world)
	{
		UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;
		UGuestSystem* guests = gameInstance ? gameInstance->GetSubsystem<UGuestSystem>() : nullptr;
		if (guests && args.Num() > 0)
		{
			guests->DebugSetReputation(FCString::Atof(*args[0]));
		}
	}));
