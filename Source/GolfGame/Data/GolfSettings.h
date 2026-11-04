// GolfSettings.h
// Project Settings > Game > Golf Game. Points the subsystems at the data registry asset.
// Requires "DeveloperSettings" in your Build.cs PublicDependencyModuleNames.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GolfSettings.generated.h"

class UGolfDataRegistry;

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
};
