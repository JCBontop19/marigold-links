#include "Interaction/GolfDesk.h"
#include "UI/GolfUIManager.h"
#include "UI/GolfPanelBase.h"
#include "Systems/DayCycleSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"

AGolfDesk::AGolfDesk()
{
	PrimaryActorTick.bCanEverTick = false;

	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(mesh);
	mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

bool AGolfDesk::CanInteract_Implementation(APawn* interactor) const
{
	if (!panelClass.Get())
	{
		return false;
	}
	if (onlyDuringClosePhase)
	{
		const UGameInstance* gameInstance = GetGameInstance();
		const UDayCycleSubsystem* dayCycle = gameInstance ? gameInstance->GetSubsystem<UDayCycleSubsystem>() : nullptr;
		return dayCycle && dayCycle->GetCurrentPhase() == EDayPhase::Close;
	}
	return true;
}

FText AGolfDesk::GetInteractPrompt_Implementation(APawn* interactor) const
{
	return CanInteract_Implementation(interactor) ? prompt : unavailablePrompt;
}

void AGolfDesk::Interact_Implementation(APawn* interactor)
{
	UGameInstance* gameInstance = GetGameInstance();
	UGolfUIManager* ui = gameInstance ? gameInstance->GetSubsystem<UGolfUIManager>() : nullptr;
	if (ui)
	{
		ui->ShowPanel(panelClass, panelContext);
	}
}
