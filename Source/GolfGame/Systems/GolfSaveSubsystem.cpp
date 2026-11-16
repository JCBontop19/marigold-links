// GolfSaveSubsystem.cpp

#include "Systems/GolfSaveSubsystem.h"
#include "Data/GolfSaveGame.h"
#include "Systems/CourseManager.h"
#include "Systems/StaffSystem.h"
#include "Systems/ClubhouseSystem.h"
#include "Systems/DayCycleSubsystem.h"
#include "Systems/GuestSystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogGolfSave, Log, All);

static const TCHAR* autoSlotName = TEXT("Auto");

void UGolfSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UCourseManager>();
	Collection.InitializeDependency<UStaffSystem>();
	Collection.InitializeDependency<UClubhouseSystem>();
	Collection.InitializeDependency<UDayCycleSubsystem>();
	Collection.InitializeDependency<UGuestSystem>();
	Super::Initialize(Collection);

	if (UGameInstance* gameInstance = GetGameInstance())
	{
		if (UDayCycleSubsystem* dayCycle = gameInstance->GetSubsystem<UDayCycleSubsystem>())
		{
			dayCycle->onDayStarted.AddDynamic(this, &UGolfSaveSubsystem::HandleDayStarted);
		}
	}
}

void UGolfSaveSubsystem::HandleDayStarted(int32 newDay)
{
	if (SaveToSlot(autoSlotName))
	{
		UE_LOG(LogGolfSave, Display, TEXT("Autosaved at the start of day %d"), newDay);
	}
}

bool UGolfSaveSubsystem::SaveToSlot(const FString& slotName)
{
	UGameInstance* gameInstance = GetGameInstance();
	if (!gameInstance)
	{
		return false;
	}

	UCourseManager* course = gameInstance->GetSubsystem<UCourseManager>();
	UStaffSystem* staff = gameInstance->GetSubsystem<UStaffSystem>();
	UClubhouseSystem* clubhouse = gameInstance->GetSubsystem<UClubhouseSystem>();
	UDayCycleSubsystem* dayCycle = gameInstance->GetSubsystem<UDayCycleSubsystem>();
	UGuestSystem* guests = gameInstance->GetSubsystem<UGuestSystem>();
	if (!course || !staff || !clubhouse || !dayCycle || !guests)
	{
		UE_LOG(LogGolfSave, Error, TEXT("Save failed: a system is missing"));
		return false;
	}

	UGolfSaveGame* save = Cast<UGolfSaveGame>(UGameplayStatics::CreateSaveGameObject(UGolfSaveGame::StaticClass()));
	if (!save)
	{
		return false;
	}

	save->savedAt = FDateTime::Now();
	course->WriteToSave(*save);
	dayCycle->WriteToSave(*save);
	staff->WriteToSave(*save);
	clubhouse->WriteToSave(*save);
	guests->WriteToSave(*save);

	const bool ok = UGameplayStatics::SaveGameToSlot(save, slotName, 0);
	UE_LOG(LogGolfSave, Display, TEXT("Save '%s': %s (day %d, cash %d)"),
		*slotName, ok ? TEXT("ok") : TEXT("FAILED"), save->currentDay, save->playerCash);
	if (ok)
	{
		onGameSaved.Broadcast(slotName);
	}
	return ok;
}

bool UGolfSaveSubsystem::LoadFromSlot(const FString& slotName)
{
	UGameInstance* gameInstance = GetGameInstance();
	if (!gameInstance || !UGameplayStatics::DoesSaveGameExist(slotName, 0))
	{
		UE_LOG(LogGolfSave, Warning, TEXT("Load '%s': no such save"), *slotName);
		return false;
	}

	const UGolfSaveGame* save = Cast<UGolfSaveGame>(UGameplayStatics::LoadGameFromSlot(slotName, 0));
	if (!save)
	{
		UE_LOG(LogGolfSave, Warning, TEXT("Load '%s': unreadable"), *slotName);
		return false;
	}
	if (save->version > UGolfSaveGame::currentVersion)
	{
		UE_LOG(LogGolfSave, Warning, TEXT("Load '%s': saved by a newer version (%d)"), *slotName, save->version);
		return false;
	}

	UCourseManager* course = gameInstance->GetSubsystem<UCourseManager>();
	UStaffSystem* staff = gameInstance->GetSubsystem<UStaffSystem>();
	UClubhouseSystem* clubhouse = gameInstance->GetSubsystem<UClubhouseSystem>();
	UDayCycleSubsystem* dayCycle = gameInstance->GetSubsystem<UDayCycleSubsystem>();
	UGuestSystem* guests = gameInstance->GetSubsystem<UGuestSystem>();
	if (!course || !staff || !clubhouse || !dayCycle || !guests)
	{
		return false;
	}

	course->ReadFromSave(*save);
	dayCycle->ReadFromSave(*save);
	staff->ReadFromSave(*save);
	clubhouse->ReadFromSave(*save);
	guests->ReadFromSave(*save);

	UE_LOG(LogGolfSave, Display, TEXT("Loaded '%s' (day %d, cash %d)"), *slotName, save->currentDay, save->playerCash);
	onGameLoaded.Broadcast();
	return true;
}

bool UGolfSaveSubsystem::DoesSaveExist(const FString& slotName) const
{
	return UGameplayStatics::DoesSaveGameExist(slotName, 0);
}

bool UGolfSaveSubsystem::DeleteSave(const FString& slotName)
{
	return UGameplayStatics::DeleteGameInSlot(slotName, 0);
}

bool UGolfSaveSubsystem::GetSaveInfo(const FString& slotName, int32& day, int32& cash, FString& savedAtLocal) const
{
	if (!UGameplayStatics::DoesSaveGameExist(slotName, 0))
	{
		return false;
	}
	const UGolfSaveGame* save = Cast<UGolfSaveGame>(UGameplayStatics::LoadGameFromSlot(slotName, 0));
	if (!save)
	{
		return false;
	}
	day = save->currentDay;
	cash = save->playerCash;
	savedAtLocal = save->savedAt.ToString(TEXT("%Y-%m-%d %H:%M"));
	return true;
}

// ---------------------------------------------------------------- Console

static UGolfSaveSubsystem* GetSaveSubsystem(UWorld* world)
{
	return (world && world->GetGameInstance()) ? world->GetGameInstance()->GetSubsystem<UGolfSaveSubsystem>() : nullptr;
}

static FAutoConsoleCommandWithWorldAndArgs GolfSaveCmd(
	TEXT("golf.Save"),
	TEXT("golf.Save [slot]: save the game (default slot Slot1)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& args, UWorld* world)
	{
		if (UGolfSaveSubsystem* saves = GetSaveSubsystem(world))
		{
			saves->SaveToSlot(args.Num() > 0 ? args[0] : TEXT("Slot1"));
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs GolfLoadCmd(
	TEXT("golf.Load"),
	TEXT("golf.Load [slot]: load the game (default slot Slot1, use Auto for the autosave)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& args, UWorld* world)
	{
		if (UGolfSaveSubsystem* saves = GetSaveSubsystem(world))
		{
			saves->LoadFromSlot(args.Num() > 0 ? args[0] : TEXT("Slot1"));
		}
	}));
