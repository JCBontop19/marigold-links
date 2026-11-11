// HoleCoverageZone.cpp

#include "Systems/HoleCoverageZone.h"
#include "Systems/CourseCoverageSubsystem.h"
#include "Systems/CourseShapeRenderer.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

AHoleCoverageZone::AHoleCoverageZone()
{
	PrimaryActorTick.bCanEverTick = false;

	box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(box);
	box->SetBoxExtent(FVector(2000.0f, 500.0f, 50.0f));   // 40 m x 10 m to start; scale or resize it
	box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	box->SetHiddenInGame(true);
	box->bDrawOnlyIfSelected = false;                       // visible in the editor so you can line it up
	box->ShapeColor = FColor(255, 200, 40);
}

void AHoleCoverageZone::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	FCourseShapeRenderer::RequestRebuild(GetWorld());
}

void AHoleCoverageZone::Destroyed()
{
	FCourseShapeRenderer::RequestRebuild(GetWorld());
	Super::Destroyed();
}

#if WITH_EDITOR
void AHoleCoverageZone::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	if (bFinished)
	{
		FCourseShapeRenderer::RequestRebuild(GetWorld());
	}
}

void AHoleCoverageZone::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);
	FCourseShapeRenderer::RequestRebuild(GetWorld());
}
#endif

void AHoleCoverageZone::RedrawCourseShapes()
{
	FCourseShapeRenderer::RebuildNow(GetWorld());
}

void AHoleCoverageZone::GetShapeParams(FVector2D& outCenter, FVector2D& outHalfSize, float& outYawDeg,
	float& outRadius, FLinearColor& outChannel) const
{
	const FVector location = GetActorLocation();
	outCenter = FVector2D(location.X, location.Y);

	const FVector scaledExtent = box ? box->GetScaledBoxExtent() : FVector(100.0f, 100.0f, 50.0f);
	outHalfSize = FVector2D(FMath::Max(1.0f, scaledExtent.X + growCm), FMath::Max(1.0f, scaledExtent.Y + growCm));

	outYawDeg = GetActorRotation().Yaw;

	const float shorterHalf = FMath::Min(outHalfSize.X, outHalfSize.Y);
	outRadius = (cornerRadiusCm < 0.0f) ? shorterHalf : FMath::Min(cornerRadiusCm, shorterHalf);

	switch (stat)
	{
	case EConditionStat::Fairway: outChannel = FLinearColor(1.0f, 0.0f, 0.0f, 0.0f); break;
	case EConditionStat::Green:   outChannel = FLinearColor(0.0f, 1.0f, 0.0f, 0.0f); break;
	default:                      outChannel = FLinearColor(0.0f, 0.0f, 1.0f, 0.0f); break;
	}
}

void AHoleCoverageZone::BeginPlay()
{
	Super::BeginPlay();

	if (!bAffectsCoverage)
	{
		return;
	}

	if (UWorld* world = GetWorld())
	{
		if (UCourseCoverageSubsystem* coverage = world->GetSubsystem<UCourseCoverageSubsystem>())
		{
			coverage->RegisterZone(this);
		}
	}
}

void AHoleCoverageZone::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	if (UWorld* world = GetWorld())
	{
		if (UCourseCoverageSubsystem* coverage = world->GetSubsystem<UCourseCoverageSubsystem>())
		{
			coverage->UnregisterZone(this);
		}
	}

	Super::EndPlay(endPlayReason);
}

void AHoleCoverageZone::GetSamplePoints(TArray<FVector2D>& outPoints) const
{
	outPoints.Reset();
	if (!box)
	{
		return;
	}

	const FVector extent = box->GetUnscaledBoxExtent();   // half-sizes in the box's local space
	const FTransform transform = box->GetComponentTransform();
	const FVector worldHalf = extent * transform.GetScale3D().GetAbs();

	const int32 countX = FMath::Max(1, FMath::FloorToInt((worldHalf.X * 2.0f) / sampleSpacingCm));
	const int32 countY = FMath::Max(1, FMath::FloorToInt((worldHalf.Y * 2.0f) / sampleSpacingCm));
	const float stepX = (extent.X * 2.0f) / countX;
	const float stepY = (extent.Y * 2.0f) / countY;

	outPoints.Reserve(countX * countY);
	for (int32 ix = 0; ix < countX; ++ix)
	{
		for (int32 iy = 0; iy < countY; ++iy)
		{
			const FVector local(-extent.X + (ix + 0.5f) * stepX, -extent.Y + (iy + 0.5f) * stepY, 0.0f);
			const FVector world = transform.TransformPosition(local);
			outPoints.Add(FVector2D(world.X, world.Y));
		}
	}
}
