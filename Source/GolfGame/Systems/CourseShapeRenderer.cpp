// CourseShapeRenderer.cpp

#include "Systems/CourseShapeRenderer.h"
#include "Systems/HoleCoverageZone.h"
#include "Data/GolfSettings.h"

#include "Containers/Ticker.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/App.h"

DEFINE_LOG_CATEGORY_STATIC(LogGolfShapes, Log, All);

namespace
{
	// +1 normally. If a rotated fairway comes out mirrored, set golf.ShapeYawSign -1.
	TAutoConsoleVariable<float> CVarYawSign(
		TEXT("golf.ShapeYawSign"), 1.0f,
		TEXT("Sign applied to zone yaw when drawing shapes (use -1 if rotated shapes are mirrored)."),
		ECVF_Default);

	FAutoConsoleCommandWithWorld CmdRebuild(
		TEXT("golf.RebuildShapes"),
		TEXT("Redraw the fairway / green shape mask now."),
		FConsoleCommandWithWorldDelegate::CreateStatic([](UWorld* world)
		{
			FCourseShapeRenderer::RebuildNow(world);
		}));

	TSet<TWeakObjectPtr<UWorld>> pendingWorlds;
}

void FCourseShapeRenderer::RequestRebuild(UWorld* world)
{
	if (!world || IsRunningCommandlet() || !FApp::CanEverRender() || world->IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	if (world->WorldType != EWorldType::Game && world->WorldType != EWorldType::PIE && world->WorldType != EWorldType::Editor)
	{
		return;
	}

	const TWeakObjectPtr<UWorld> weakWorld(world);
	if (pendingWorlds.Contains(weakWorld))
	{
		return;
	}
	pendingWorlds.Add(weakWorld);

	// The core ticker runs in the editor as well as in game, unlike a world timer.
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([weakWorld](float) -> bool
	{
		pendingWorlds.Remove(weakWorld);
		if (UWorld* liveWorld = weakWorld.Get())
		{
			FCourseShapeRenderer::RebuildNow(liveWorld);
		}
		return false;   // run once
	}));
}

void FCourseShapeRenderer::RebuildNow(UWorld* world)
{
	if (!world)
	{
		return;
	}

	const UGolfSettings* settings = GetDefault<UGolfSettings>();
	UTextureRenderTarget2D* target = settings->shapeMaskTarget.LoadSynchronous();
	UMaterialInterface* stamp = settings->shapeStampMaterial.LoadSynchronous();
	if (!target || !stamp || settings->maskSize <= 1.0f)
	{
		UE_LOG(LogGolfShapes, Warning,
			TEXT("Shape mask not drawn: set Shape Mask Target and Shape Stamp Material in Project Settings > Golf Game."));
		return;
	}

	UKismetRenderingLibrary::ClearRenderTarget2D(world, target, FLinearColor::Black);

	UCanvas* canvas = nullptr;
	FVector2D canvasSize = FVector2D::ZeroVector;
	FDrawToRenderTargetContext context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(world, target, canvas, canvasSize, context);
	if (!canvas)
	{
		return;
	}

	const double pxPerCm = canvasSize.X / settings->maskSize;
	const float yawSign = CVarYawSign.GetValueOnGameThread() < 0.0f ? -1.0f : 1.0f;
	const double padPx = 2.0;   // room around the shape so the soft edge is not clipped

	int32 drawn = 0;
	for (TActorIterator<AHoleCoverageZone> it(world); it; ++it)
	{
		const AHoleCoverageZone* zone = *it;
		if (!IsValid(zone) || !zone->bDrawShape)
		{
			continue;
		}

		FVector2D center;
		FVector2D halfSize;
		float yawDeg = 0.0f;
		float radius = 0.0f;
		FLinearColor channel;
		zone->GetShapeParams(center, halfSize, yawDeg, radius, channel);

		const FVector2D centerPx = (center - settings->maskOrigin) * pxPerCm;
		const FVector2D halfPx = halfSize * pxPerCm;
		const FVector2D quadPx = (halfPx + FVector2D(padPx, padPx)) * 2.0;

		UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(stamp, GetTransientPackage());
		material->SetVectorParameterValue(TEXT("QuadSize"), FLinearColor(quadPx.X, quadPx.Y, 0.0f, 0.0f));
		material->SetVectorParameterValue(TEXT("HalfSize"), FLinearColor(halfPx.X, halfPx.Y, 0.0f, 0.0f));
		material->SetScalarParameterValue(TEXT("Radius"), radius * pxPerCm);
		material->SetVectorParameterValue(TEXT("Channel"), channel);

		canvas->K2_DrawMaterial(material, centerPx - quadPx * 0.5, quadPx,
			FVector2D(0.0, 0.0), FVector2D(1.0, 1.0), yawDeg * yawSign, FVector2D(0.5, 0.5));
		++drawn;
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(world, context);
	UE_LOG(LogGolfShapes, Log, TEXT("Drew %d course shapes."), drawn);
}
