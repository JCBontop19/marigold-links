// GolfSettings.h
// Project Settings > Game > Golf Game. Points the subsystems at the data registry asset
// and at the course-shape mask assets.
// Requires "DeveloperSettings" in your Build.cs PublicDependencyModuleNames.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GolfSettings.generated.h"

class UGolfDataRegistry;
class UTextureRenderTarget2D;
class UMaterialInterface;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Golf Game"))
class GOLFGAME_API UGolfSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGolfSettings()
	{
		CategoryName = TEXT("Game");
	}

	UPROPERTY(Config, EditAnywhere, Category = "Data")
	TSoftObjectPtr<UGolfDataRegistry> dataRegistry;

	// ---- Course shapes (fairway / green mask drawn from HoleCoverageZone boxes) ----

	// Render target the shapes are drawn into (RGBA8, 4096 x 4096). R = fairway, G = green, B = rough.
	UPROPERTY(Config, EditAnywhere, Category = "Course Shapes")
	TSoftObjectPtr<UTextureRenderTarget2D> shapeMaskTarget;

	// User Interface / Additive material that stamps one rounded rectangle (M_ShapeStamp).
	UPROPERTY(Config, EditAnywhere, Category = "Course Shapes")
	TSoftObjectPtr<UMaterialInterface> shapeStampMaterial;

	// World XY (cm) of the mask's top-left corner. Same as MowOrigin in MPC_Course.
	UPROPERTY(Config, EditAnywhere, Category = "Course Shapes")
	FVector2D maskOrigin = FVector2D(-5000.0, -5000.0);

	// World size (cm) the mask covers. Same as MowSize in MPC_Course.
	UPROPERTY(Config, EditAnywhere, Category = "Course Shapes")
	float maskSize = 63000.0f;
};
