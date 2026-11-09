// HoleCoverageZone.cpp

#include "Systems/HoleCoverageZone.h"
#include "Systems/CourseCoverageSubsystem.h"
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

void AHoleCoverageZone::BeginPlay()
{
	Super::BeginPlay();

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
