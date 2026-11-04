#include "Interaction/InteractionComponent.h"
#include "Interaction/Interactable.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

AActor* UInteractionComponent::FindInteractableInView(APawn* pawn) const
{
	AController* controller = pawn ? pawn->GetController() : nullptr;
	UWorld* world = GetWorld();
	if (!controller || !world)
	{
		return nullptr;
	}

	FVector viewLocation;
	FRotator viewRotation;
	controller->GetPlayerViewPoint(viewLocation, viewRotation);

	const FVector traceEnd = viewLocation + viewRotation.Vector() * interactRange;

	FCollisionQueryParams params(SCENE_QUERY_STAT(GolfInteract), false, pawn);
	FHitResult hit;
	if (!world->LineTraceSingleByChannel(hit, viewLocation, traceEnd, traceChannel, params))
	{
		return nullptr;
	}

	AActor* hitActor = hit.GetActor();
	if (hitActor && hitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return hitActor;
	}
	return nullptr;
}

void UInteractionComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	APawn* pawn = Cast<APawn>(GetOwner());
	if (!pawn || !pawn->IsLocallyControlled())
	{
		return;
	}

	AActor* target = FindInteractableInView(pawn);

	FText newPrompt;
	if (target)
	{
		newPrompt = IInteractable::Execute_GetInteractPrompt(target, pawn);
	}

	if (target != focusedActor.Get() || !newPrompt.EqualTo(currentPrompt))
	{
		focusedActor = target;
		currentPrompt = newPrompt;
		onFocusChanged.Broadcast(target, newPrompt);
	}
}

bool UInteractionComponent::TryInteract()
{
	APawn* pawn = Cast<APawn>(GetOwner());
	AActor* target = focusedActor.Get();
	if (!pawn || !target)
	{
		return false;
	}
	if (!IInteractable::Execute_CanInteract(target, pawn))
	{
		return false;
	}
	IInteractable::Execute_Interact(target, pawn);
	return true;
}
