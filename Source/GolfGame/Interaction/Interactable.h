// Interactable.h
// Anything the player can look at and press E on implements this (C++ or Blueprint).

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(BlueprintType, Blueprintable)
class GOLFGAME_API UInteractable : public UInterface
{
	GENERATED_BODY()
};

class GOLFGAME_API IInteractable
{
	GENERATED_BODY()

public:
	// False = the prompt still shows (see GetInteractPrompt) but pressing E does nothing.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(APawn* interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractPrompt(APawn* interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(APawn* interactor);
};
