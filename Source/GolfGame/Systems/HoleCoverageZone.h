// HoleCoverageZone.h
// Place one or more of these over the mowable parts of a hole (fairway strips, the green).
// At day close, CourseCoverageSubsystem samples the cut mask inside each box and turns
// "how much of this area is mowed" into a condition change for the hole.
//
// Several zones may share a hole number and stat: their samples are pooled, so a curved
// fairway can be covered by a few rotated boxes.
//
// The same box is also drawn as a rounded rectangle into the course shape mask
// (see CourseShapeRenderer), which M_Turf uses for the fairway / green look.
// Keep each box rotated around Z only (Yaw).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/GolfTypes.h"
#include "HoleCoverageZone.generated.h"

class UBoxComponent;

UCLASS()
class GOLFGAME_API AHoleCoverageZone : public AActor
{
	GENERATED_BODY()

public:
	AHoleCoverageZone();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coverage")
	int32 holeNumber = 1;

	// Which condition this area feeds (Fairway strips, Green disc, Rough fringe).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coverage")
	EConditionStat stat = EConditionStat::Fairway;

	// Distance between sample points, in cm. Smaller = more accurate, more samples.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coverage", meta = (ClampMin = "25.0"))
	float sampleSpacingCm = 200.0f;

	// Turn off to use the box only for the shape (for example a fringe ring around a green).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coverage")
	bool bAffectsCoverage = true;

	// ---- Shape drawn into the course mask ----

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape")
	bool bDrawShape = true;

	// Corner radius in cm. -1 = automatic: half of the shorter side, so the ends are half circles
	// (a square box becomes a circle). 0 = sharp corners (tee boxes).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (ClampMin = "-1.0"))
	float cornerRadiusCm = -1.0f;

	// Makes the drawn shape bigger (or smaller, if negative) than the box, in cm on every side.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape")
	float growCm = 0.0f;

	// Redraws the shape mask now (it also redraws by itself when a zone is moved or edited).
	UFUNCTION(CallInEditor, Category = "Shape")
	void RedrawCourseShapes();

	// Centre and half-size (cm), yaw (degrees), corner radius (cm) and mask channel for this zone.
	void GetShapeParams(FVector2D& outCenter, FVector2D& outHalfSize, float& outYawDeg,
		float& outRadius, FLinearColor& outChannel) const;

	// World-space XY points (cm) spread over the box, for the subsystem to look up in the cut mask.
	void GetSamplePoints(TArray<FVector2D>& outPoints) const;

protected:
	virtual void PostRegisterAllComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void Destroyed() override;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& event) override;
#endif

private:
	UPROPERTY(VisibleAnywhere, Category = "Coverage")
	TObjectPtr<UBoxComponent> box;
};
