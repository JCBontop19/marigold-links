// CourseCoverageSubsystem.cpp

#include "Systems/CourseCoverageSubsystem.h"
#include "Systems/HoleCoverageZone.h"
#include "Systems/CourseManager.h"
#include "Systems/DayCycleSubsystem.h"
#include "Data/GolfData.h"
#include "Data/GolfSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "TextureResource.h"

DEFINE_LOG_CATEGORY_STATIC(LogGolfCoverage, Log, All);

bool UCourseCoverageSubsystem::ShouldCreateSubsystem(UObject* outer) const
{
	const UWorld* world = Cast<UWorld>(outer);
	return world && world->IsGameWorld();
}

void UCourseCoverageSubsystem::OnWorldBeginPlay(UWorld& inWorld)
{
	Super::OnWorldBeginPlay(inWorld);

	if (UGameInstance* gameInstance = inWorld.GetGameInstance())
	{
		if (UDayCycleSubsystem* dayCycle = gameInstance->GetSubsystem<UDayCycleSubsystem>())
		{
			dayCycle->onPhaseChanged.AddDynamic(this, &UCourseCoverageSubsystem::HandlePhaseChanged);
		}
	}
}

void UCourseCoverageSubsystem::Deinitialize()
{
	if (UWorld* world = GetWorld())
	{
		if (UGameInstance* gameInstance = world->GetGameInstance())
		{
			if (UDayCycleSubsystem* dayCycle = gameInstance->GetSubsystem<UDayCycleSubsystem>())
			{
				dayCycle->onPhaseChanged.RemoveDynamic(this, &UCourseCoverageSubsystem::HandlePhaseChanged);
			}
		}
	}

	zones.Reset();
	Super::Deinitialize();
}

void UCourseCoverageSubsystem::Configure(UTextureRenderTarget2D* inCutMask, FVector2D inMowOrigin, float inMowSize)
{
	cutMask = inCutMask;
	mowOrigin = inMowOrigin;
	mowSize = FMath::Max(inMowSize, 1.0f);
}

void UCourseCoverageSubsystem::RegisterZone(AHoleCoverageZone* zone)
{
	if (zone)
	{
		zones.AddUnique(zone);
	}
}

void UCourseCoverageSubsystem::UnregisterZone(AHoleCoverageZone* zone)
{
	zones.Remove(zone);
}

void UCourseCoverageSubsystem::HandlePhaseChanged(EDayPhase newPhase)
{
	if (newPhase == EDayPhase::Close)
	{
		ApplyCoverageToCourse(false);
	}
}

float UCourseCoverageSubsystem::GetLastCoverage(int32 holeNumber, EConditionStat stat) const
{
	const float* found = lastCoverage.Find(MakeKey(holeNumber, stat));
	return found ? *found : 0.0f;
}

float UCourseCoverageSubsystem::GetRestorePerFullMow(EConditionStat stat, int32 ownedTier) const
{
	const UGolfSettings* settings = GetDefault<UGolfSettings>();
	const UGolfDataRegistry* registry = settings ? settings->dataRegistry.LoadSynchronous() : nullptr;
	if (!registry)
	{
		return 0.10f;
	}

	float bestForStat = 0.0f;
	float baseMower = 0.10f;   // tier 0 (push mower) restore, used as the fallback

	for (const UEquipmentData* equipment : registry->equipment)
	{
		if (!equipment || equipment->tier > ownedTier)
		{
			continue;
		}
		if (equipment->tier == 0)
		{
			baseMower = equipment->restoreAmount;
		}
		if (equipment->affectsStat == stat)
		{
			bestForStat = FMath::Max(bestForStat, equipment->restoreAmount);
		}
	}

	// No tool owned for this stat yet (for example a green before the aerator): the push mower
	// still helps a little, at half strength.
	return bestForStat > 0.0f ? bestForStat : baseMower * 0.5f;
}

int32 UCourseCoverageSubsystem::ApplyCoverageToCourse(bool force)
{
	UWorld* world = GetWorld();
	UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;
	if (!gameInstance || !cutMask || zones.Num() == 0)
	{
		return 0;
	}

	UCourseManager* course = gameInstance->GetSubsystem<UCourseManager>();
	UDayCycleSubsystem* dayCycle = gameInstance->GetSubsystem<UDayCycleSubsystem>();
	if (!course)
	{
		return 0;
	}

	const int32 today = dayCycle ? dayCycle->GetCurrentDay() : 0;
	if (!force && today == lastAppliedDay)
	{
		return 0;   // already applied for this day
	}

	FTextureRenderTargetResource* resource = cutMask->GameThread_GetRenderTargetResource();
	if (!resource)
	{
		return 0;
	}

	// One read of the whole mask. About 64 MB for 4096 x 4096, once a day at close.
	TArray<FColor> pixels;
	FReadSurfaceDataFlags readFlags(RCM_UNorm);
	readFlags.SetLinearToGamma(false);
	if (!resource->ReadPixels(pixels, readFlags))
	{
		UE_LOG(LogGolfCoverage, Warning, TEXT("Could not read the cut mask."));
		return 0;
	}

	const int32 width = cutMask->SizeX;
	const int32 height = cutMask->SizeY;
	if (width <= 0 || height <= 0 || pixels.Num() != width * height)
	{
		UE_LOG(LogGolfCoverage, Warning, TEXT("Cut mask size mismatch (%d pixels for %dx%d)."), pixels.Num(), width, height);
		return 0;
	}

	// Pool samples per (hole, stat) so several zones can cover one hole.
	struct FAccumulator
	{
		double sum = 0.0;
		int32 count = 0;
	};
	TMap<int32, FAccumulator> accumulators;

	int32 measuredZones = 0;
	for (const TWeakObjectPtr<AHoleCoverageZone>& weakZone : zones)
	{
		const AHoleCoverageZone* zone = weakZone.Get();
		if (!zone)
		{
			continue;
		}
		++measuredZones;

		TArray<FVector2D> points;
		zone->GetSamplePoints(points);

		FAccumulator& accumulator = accumulators.FindOrAdd(MakeKey(zone->holeNumber, zone->stat));
		for (const FVector2D& point : points)
		{
			const double u = (point.X - mowOrigin.X) / mowSize;
			const double v = (point.Y - mowOrigin.Y) / mowSize;

			float cut = 0.0f;   // outside the mask counts as uncut
			if (u >= 0.0 && u < 1.0 && v >= 0.0 && v < 1.0)
			{
				const int32 px = FMath::Clamp(FMath::FloorToInt(u * width), 0, width - 1);
				const int32 py = FMath::Clamp(FMath::FloorToInt(v * height), 0, height - 1);
				cut = pixels[py * width + px].R / 255.0f;   // R channel = cut amount
			}
			accumulator.sum += cut;
			++accumulator.count;
		}
	}

	const int32 ownedTier = course->GetOwnedEquipmentTier();
	for (const TPair<int32, FAccumulator>& entry : accumulators)
	{
		if (entry.Value.count == 0)
		{
			continue;
		}

		const int32 holeNumber = entry.Key / 10;
		const EConditionStat stat = static_cast<EConditionStat>(entry.Key % 10);
		const float coverage = static_cast<float>(entry.Value.sum / entry.Value.count);
		lastCoverage.Add(entry.Key, coverage);

		if (!course->IsHoleUnlocked(holeNumber))
		{
			continue;   // locked holes are not maintained
		}

		const float restore = coverage * GetRestorePerFullMow(stat, ownedTier);
		course->ApplyConditionChange(holeNumber, stat, restore);

		UE_LOG(LogGolfCoverage, Display, TEXT("Hole %d %s: coverage %.2f -> +%.3f condition"),
			holeNumber, *UEnum::GetValueAsString(stat), coverage, restore);
	}

	lastAppliedDay = today;
	return measuredZones;
}
