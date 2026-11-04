#include "UI/GolfPanelBase.h"
#include "UI/GolfUIManager.h"
#include "Systems/CourseManager.h"
#include "Systems/StaffSystem.h"
#include "Systems/DayCycleSubsystem.h"
#include "Systems/ClubhouseSystem.h"
#include "Data/GolfData.h"
#include "Data/GolfSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UGolfPanelBase::InitPanel(UObject* inContext)
{
	panelContext = inContext;
	OnPanelOpened(inContext);
}

void UGolfPanelBase::ClosePanel()
{
	if (UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UGolfUIManager* ui = gameInstance->GetSubsystem<UGolfUIManager>())
		{
			ui->CloseActivePanel();
		}
	}
}

UCourseManager* UGolfPanelBase::GetCourseManager() const
{
	UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(this);
	return gameInstance ? gameInstance->GetSubsystem<UCourseManager>() : nullptr;
}

UStaffSystem* UGolfPanelBase::GetStaffSystem() const
{
	UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(this);
	return gameInstance ? gameInstance->GetSubsystem<UStaffSystem>() : nullptr;
}

UDayCycleSubsystem* UGolfPanelBase::GetDayCycle() const
{
	UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(this);
	return gameInstance ? gameInstance->GetSubsystem<UDayCycleSubsystem>() : nullptr;
}

UClubhouseSystem* UGolfPanelBase::GetClubhouseSystem() const
{
	UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(this);
	return gameInstance ? gameInstance->GetSubsystem<UClubhouseSystem>() : nullptr;
}

TArray<UHoleData*> UGolfPanelBase::GetLockedHoles() const
{
	TArray<UHoleData*> result;
	UCourseManager* course = GetCourseManager();
	if (!course)
	{
		return result;
	}

	for (UHoleData* hole : course->GetAllHoleData())
	{
		if (hole && hole->countsForRevenue && !course->IsHoleUnlocked(hole->holeNumber))
		{
			result.Add(hole);
		}
	}
	return result;
}

TArray<UEquipmentData*> UGolfPanelBase::GetEquipmentForSale() const
{
	TArray<UEquipmentData*> result;
	const UGolfSettings* settings = GetDefault<UGolfSettings>();
	UGolfDataRegistry* registry = settings ? settings->dataRegistry.LoadSynchronous() : nullptr;
	if (!registry)
	{
		return result;
	}

	for (UEquipmentData* equipment : registry->equipment)
	{
		if (equipment)
		{
			result.Add(equipment);
		}
	}
	result.Sort([](const UEquipmentData& a, const UEquipmentData& b) { return a.tier < b.tier; });
	return result;
}

void UGolfPanelBase::ResolveApplication(UApplicationPairData* pair, bool hireA)
{
	UStaffSystem* staff = GetStaffSystem();
	if (!pair || !staff)
	{
		return;
	}

	UStaffData* chosen = hireA ? pair->applicantA : pair->applicantB;
	UStaffData* rejected = hireA ? pair->applicantB : pair->applicantA;

	staff->HireStaff(chosen);
	staff->RejectApplicant(rejected);
	ClosePanel();
}

void UGolfPanelBase::CloseReportAndEndDay()
{
	ClosePanel();
	if (UDayCycleSubsystem* dayCycle = GetDayCycle())
	{
		dayCycle->CloseOutDay();
	}
}
