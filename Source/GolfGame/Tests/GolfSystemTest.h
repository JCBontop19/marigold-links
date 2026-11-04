// GolfSystemTest.h
// Replacement for test.gd. Drop one of these into any level, assign Jake and Maria,
// press Play, and read the Output Log (filter: GolfTest). Every line is PASS or FAIL.
// Delete this once real UI exercises the systems.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GolfSystemTest.generated.h"

class UStaffData;

UCLASS()
class GOLFGAME_API AGolfSystemTest : public AActor
{
	GENERATED_BODY()

public:
	AGolfSystemTest();

	// Jake: role Groundskeeper, skill 0.2, daily wage 35, hire cost 80
	UPROPERTY(EditAnywhere, Category = "Test")
	TObjectPtr<UStaffData> jake;

	// Maria: role Groundskeeper, skill 0.8, daily wage 70, hire cost 200
	UPROPERTY(EditAnywhere, Category = "Test")
	TObjectPtr<UStaffData> maria;

protected:
	virtual void BeginPlay() override;

private:
	void RunCourseTests();
	void RunStaffTests();
	void RunDayCycleTests();
	void RunClubhouseTests();

	void ExpectTrue(const FString& label, bool condition);
	void ExpectInt(const FString& label, int32 expected, int32 actual);
	void ExpectFloat(const FString& label, float expected, float actual);

	UFUNCTION()
	void HandleMidnightPenalty();

	int32 passCount = 0;
	int32 failCount = 0;
	bool penaltySignalFired = false;
};
