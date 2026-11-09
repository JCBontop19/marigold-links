// HoleCoverageZone.h
// Place one or more of these over the mowable parts of a hole (fairway strips, the green).
// At day close, CourseCoverageSubsystem samples the cut mask inside each box and turns
// "how much of this area is mowed" into a condition change for the hole.
//
// Several zones may share a hole number and stat: their samples are pooled, so a curved
// fairway can be covered by a few rotated boxes.

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

	// World-space XY points (cm) spread over the box, for the subsystem to look up in the cut mask.
	void GetSamplePoints(TArray<FVector2D>& outPoints) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Coverage")
	TObjectPtr<UBoxComponent> box;
};
