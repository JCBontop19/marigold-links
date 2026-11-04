// DayCycleSubsystem.cpp

#include "Systems/DayCycleSubsystem.h"
#include "Systems/CourseManager.h"
#include "Systems/StaffSystem.h"
#include "Engine/GameInstance.h"

void UDayCycleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UCourseManager>();
	Collection.InitializeDependency<UStaffSystem>();
	Super::Initialize(Collection);

	currentDay = 1;
	currentTimeMinutes = dayStartMinutes;
	closeElapsedMinutes = 0.0f;
	currentPhase = EDayPhase::MorningPrep;
	lastReport = FDayReport();
	tickAccumulator = 0.0f;
}

void UDayCycleSubsystem::Tick(float deltaTime)
{
	if (tickIntervalSeconds <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	tickAccumulator += deltaTime;
	while (tickAccumulator >= tickIntervalSeconds)
	{
		tickAccumulator -= tickIntervalSeconds;
		AdvanceOneTick();
	}
}

void UDayCycleSubsystem::AdvanceOneTick()
{
	if (currentPhase != EDayPhase::Close)
	{
		currentTimeMinutes += minutesPerTick;

		if (currentTimeMinutes >= closeStartMinutes)
		{
			SetPhase(EDayPhase::Close);
			RunCloseProcessing();
		}
		else if (currentTimeMinutes >= openStartMinutes)
		{
			if (currentPhase != EDayPhase::OpenHours)
			{
				SetPhase(EDayPhase::OpenHours);
			}
		}
		else
		{
			if (currentPhase != EDayPhase::MorningPrep)
			{
				SetPhase(EDayPhase::MorningPrep);
			}
		}
	}
	else
	{
		// The clock is frozen during Close; only the midnight countdown advances.
		closeElapsedMinutes += minutesPerTick;
		if (closeElapsedMinutes >= closeDeadlineMinutes)
		{
			TriggerMidnightPenalty();
		}
	}
}

void UDayCycleSubsystem::SetPhase(EDayPhase newPhase)
{
	currentPhase = newPhase;
	onPhaseChanged.Broadcast(currentPhase);
}

void UDayCycleSubsystem::RunCloseProcessing()
{
	UGameInstance* gameInstance = GetGameInstance();
	UCourseManager* course = gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
	UStaffSystem* staff = gameInstance ? gameInstance->GetSubsystem<UStaffSystem>() : nullptr;
	if (!course || !staff)
	{
		UE_LOG(LogTemp, Error, TEXT("DayCycle: missing CourseManager or StaffSystem during close processing"));
		return;
	}

	const int32 revenue = course->GetRevenueHoleCount() * baseDailyRevenue;
	const int32 wages = staff->GetTotalWages();

	// Maintenance first, then natural decay (order matters because conditions clamp at 1.0).
	const TMap<int32, float> effects = staff->GetMaintenanceEffects();
	for (const TPair<int32, float>& effect : effects)
	{
		course->ApplyConditionChange(effect.Key, EConditionStat::Fairway, effect.Value);
		course->ApplyConditionChange(effect.Key, EConditionStat::Rough, effect.Value);
		course->ApplyConditionChange(effect.Key, EConditionStat::Green, effect.Value);
	}

	for (const int32 holeNumber : course->GetUnlockedHoleNumbers())
	{
		course->ApplyConditionChange(holeNumber, EConditionStat::Fairway, naturalDecayAmount);
		course->ApplyConditionChange(holeNumber, EConditionStat::Rough, naturalDecayAmount);
		course->ApplyConditionChange(holeNumber, EConditionStat::Green, naturalDecayAmount);
	}

	const int32 net = revenue - wages;
	course->AddCash(net);

	FDayReport report;
	report.day = currentDay;
	report.revenue = revenue;
	report.wages = wages;
	report.net = net;
	lastReport = report;

	onDayReportReady.Broadcast(lastReport);
	onDayEnded.Broadcast();
}

void UDayCycleSubsystem::TriggerMidnightPenalty()
{
	onMidnightPenaltyTriggered.Broadcast();

	UGameInstance* gameInstance = GetGameInstance();
	if (UCourseManager* course = gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr)
	{
		course->AddCash(-midnightCashPenalty);
	}
	if (UStaffSystem* staff = gameInstance ? gameInstance->GetSubsystem<UStaffSystem>() : nullptr)
	{
		staff->ApplyMoralePenalty(midnightMoralePenalty);
	}

	CloseOutDay();
}

bool UDayCycleSubsystem::CloseOutDay()
{
	if (currentPhase != EDayPhase::Close)
	{
		UE_LOG(LogTemp, Warning, TEXT("DayCycle: CloseOutDay called outside the Close phase"));
		return false;
	}

	++currentDay;
	currentTimeMinutes = dayStartMinutes;
	closeElapsedMinutes = 0.0f;
	tickAccumulator = 0.0f;
	SetPhase(EDayPhase::MorningPrep);
	onDayStarted.Broadcast(currentDay);
	return true;
}

FString UDayCycleSubsystem::GetFormattedTime() const
{
	const int32 totalMinutes = FMath::FloorToInt(currentTimeMinutes) % 1440;
	const int32 hour24 = totalMinutes / 60;
	const int32 minute = totalMinutes % 60;
	const int32 hour12 = (hour24 % 12 == 0) ? 12 : (hour24 % 12);
	return FString::Printf(TEXT("%d:%02d %s"), hour12, minute, hour24 < 12 ? TEXT("AM") : TEXT("PM"));
}
