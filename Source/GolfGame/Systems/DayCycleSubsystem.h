// DayCycleSubsystem.h
// Port of the DayCycle autoload: real-time clock, phases, close processing, midnight penalty.
//
// It is a GameInstance subsystem (so state survives Open Level, e.g. entering the clubhouse
// interior) that also ticks via FTickableGameObject. It does NOT tick while the game is paused,
// so opening any panel that calls SetGamePaused(true) stops the clock automatically.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Data/GolfTypes.h"
#include "DayCycleSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EDayPhase, newPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayReportReady, FDayReport, report);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDayEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayStarted, int32, newDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMidnightPenaltyTriggered);

UCLASS()
class GOLFGAME_API UDayCycleSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FTickableGameObject
	virtual void Tick(float deltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UDayCycleSubsystem, STATGROUP_Tickables); }
	virtual bool IsTickable() const override { return !IsTemplate(); }
	virtual bool IsTickableWhenPaused() const override { return false; }

	// --- Clock constants (minutes since midnight) ---
	static constexpr float dayStartMinutes = 300.0f;      // 5:00 AM
	static constexpr float openStartMinutes = 420.0f;     // 7:00 AM
	static constexpr float closeStartMinutes = 1140.0f;   // 7:00 PM
	static constexpr int32 minutesPerTick = 15;
	static constexpr float closeDeadlineMinutes = 300.0f; // close-phase minutes until midnight penalty

	// --- Economy constants ---
	static constexpr int32 baseDailyRevenue = 200;        // per unlocked hole (placeholder until guests exist)
	static constexpr float naturalDecayAmount = -0.05f;
	static constexpr int32 midnightCashPenalty = 100;
	static constexpr float midnightMoralePenalty = 0.15f;

	// Real seconds per clock tick. Lower it to test quickly.
	UPROPERTY(BlueprintReadWrite, Category = "Day Cycle")
	float tickIntervalSeconds = 10.0f;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Day Cycle")
	FOnPhaseChanged onPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Day Cycle")
	FOnDayReportReady onDayReportReady;

	UPROPERTY(BlueprintAssignable, Category = "Day Cycle")
	FOnDayEnded onDayEnded;

	UPROPERTY(BlueprintAssignable, Category = "Day Cycle")
	FOnDayStarted onDayStarted;

	UPROPERTY(BlueprintAssignable, Category = "Day Cycle")
	FOnMidnightPenaltyTriggered onMidnightPenaltyTriggered;

	// --- Queries ---
	UFUNCTION(BlueprintPure, Category = "Day Cycle")
	int32 GetCurrentDay() const { return currentDay; }

	UFUNCTION(BlueprintPure, Category = "Day Cycle")
	EDayPhase GetCurrentPhase() const { return currentPhase; }

	UFUNCTION(BlueprintPure, Category = "Day Cycle")
	float GetCurrentTimeMinutes() const { return currentTimeMinutes; }

	UFUNCTION(BlueprintPure, Category = "Day Cycle")
	float GetCloseElapsedMinutes() const { return closeElapsedMinutes; }

	UFUNCTION(BlueprintPure, Category = "Day Cycle")
	float GetCloseMinutesRemaining() const { return FMath::Max(0.0f, closeDeadlineMinutes - closeElapsedMinutes); }

	UFUNCTION(BlueprintPure, Category = "Day Cycle")
	FDayReport GetLastReport() const { return lastReport; }

	// "5:00 AM" style, 15-minute steps.
	UFUNCTION(BlueprintPure, Category = "Day Cycle")
	FString GetFormattedTime() const;

	// --- Actions ---
	// Advances exactly one 15-minute tick. Tick() calls this; tests can call it directly.
	UFUNCTION(BlueprintCallable, Category = "Day Cycle")
	void AdvanceOneTick();

	// Called when the player closes the report. Only valid during the Close phase.
	UFUNCTION(BlueprintCallable, Category = "Day Cycle")
	bool CloseOutDay();

private:
	void SetPhase(EDayPhase newPhase);
	void RunCloseProcessing();
	void TriggerMidnightPenalty();

	UPROPERTY()
	int32 currentDay = 1;

	UPROPERTY()
	float currentTimeMinutes = dayStartMinutes;

	UPROPERTY()
	float closeElapsedMinutes = 0.0f;

	UPROPERTY()
	EDayPhase currentPhase = EDayPhase::MorningPrep;

	UPROPERTY()
	FDayReport lastReport;

	float tickAccumulator = 0.0f;
};
