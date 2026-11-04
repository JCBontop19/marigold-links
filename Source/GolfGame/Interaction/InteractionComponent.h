// InteractionComponent.h
// Add to the player pawn. Traces from the camera each frame, tracks the interactable
// being looked at, and broadcasts a prompt for the HUD. Call TryInteract() from the
// Interact (E) input action in the pawn Blueprint.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractFocusChanged, AActor*, focusedActor, FText, prompt);

UCLASS(ClassGroup = (Golf), meta = (BlueprintSpawnableComponent))
class GOLFGAME_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float interactRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> traceChannel = ECC_Visibility;

	// Fires when the focused actor or its prompt text changes. focusedActor is null (and prompt empty)
	// when nothing is in view.
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractFocusChanged onFocusChanged;

	// Returns true if an interaction happened.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedActor() const { return focusedActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetCurrentPrompt() const { return currentPrompt; }

private:
	AActor* FindInteractableInView(APawn* pawn) const;

	TWeakObjectPtr<AActor> focusedActor;
	FText currentPrompt;
};
