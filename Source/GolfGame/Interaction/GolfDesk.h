// GolfDesk.h
// A placeable interactable that opens a panel. Make one Blueprint child per desk
// (report / applications / hole unlock / hardware store) and set the properties in Details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "GolfDesk.generated.h"

class UGolfPanelBase;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class GOLFGAME_API AGolfDesk : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AGolfDesk();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
	TObjectPtr<UStaticMeshComponent> mesh;

	// Widget Blueprint (child of GolfPanelBase) to open.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk")
	TSubclassOf<UGolfPanelBase> panelClass;

	// Passed to the panel's OnPanelOpened (e.g. an ApplicationPairData asset). Optional.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk")
	TObjectPtr<UObject> panelContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk")
	FText prompt = FText::FromString(TEXT("Press E to interact"));

	// Shown instead of prompt when the desk is currently unavailable.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk")
	FText unavailablePrompt = FText::FromString(TEXT("Not available right now"));

	// Report desk: only usable once the course has closed for the day.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desk")
	bool onlyDuringClosePhase = false;

	virtual bool CanInteract_Implementation(APawn* interactor) const override;
	virtual FText GetInteractPrompt_Implementation(APawn* interactor) const override;
	virtual void Interact_Implementation(APawn* interactor) override;
};
