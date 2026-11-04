// StaffSystem.h
// Port of the StaffSystem autoload. Roster, hiring, assignment, wages, morale.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/GolfTypes.h"
#include "StaffSystem.generated.h"

class UStaffData;
class UCourseManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRosterChanged);

UCLASS()
class GOLFGAME_API UStaffSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Base restore per assigned groundskeeper: baseRestore * (1 + skillLevel).
	static constexpr float baseRestore = 0.10f;

	UPROPERTY(BlueprintAssignable, Category = "Staff")
	FOnRosterChanged onRosterChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Staff")
	TArray<FStaffState> roster;

	UPROPERTY(BlueprintReadOnly, Category = "Staff")
	TArray<TObjectPtr<UStaffData>> rejectedApplicants;

	// Pays the hire cost. Fails if already hired or the player cannot afford it.
	UFUNCTION(BlueprintCallable, Category = "Staff")
	bool HireStaff(UStaffData* staff);

	UFUNCTION(BlueprintCallable, Category = "Staff")
	bool FireStaff(UStaffData* staff);

	// Pass -1 to unassign. Fails if the staff member is not hired or the hole is locked.
	UFUNCTION(BlueprintCallable, Category = "Staff")
	bool AssignToHole(UStaffData* staff, int32 holeNumber);

	UFUNCTION(BlueprintCallable, Category = "Staff")
	void RejectApplicant(UStaffData* staff);

	UFUNCTION(BlueprintPure, Category = "Staff")
	int32 GetTotalWages() const;

	UFUNCTION(BlueprintCallable, Category = "Staff")
	void ApplyMoralePenalty(float amount);

	UFUNCTION(BlueprintPure, Category = "Staff")
	bool IsHired(const UStaffData* staff) const;

	// Restore amount for one hole, summed over assigned groundskeepers.
	UFUNCTION(BlueprintPure, Category = "Staff")
	float GetMaintenanceForHole(int32 holeNumber) const;

	// Hole number -> total restore for every hole that has at least one groundskeeper.
	TMap<int32, float> GetMaintenanceEffects() const;

	// Native lookup (used by tests and other systems).
	FStaffState* FindStaffState(const UStaffData* staff);
	const FStaffState* FindStaffState(const UStaffData* staff) const;

private:
	UCourseManager* GetCourse() const;
};
