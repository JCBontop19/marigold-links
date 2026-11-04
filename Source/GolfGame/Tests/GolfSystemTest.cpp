// GolfSystemTest.cpp
// Mirrors test.gd's verified sequence and expected values. Run once per Play session:
// the subsystems reset every time PIE starts, and this test spends cash and unlocks holes.

#include "Tests/GolfSystemTest.h"
#include "Systems/CourseManager.h"
#include "Systems/StaffSystem.h"
#include "Systems/DayCycleSubsystem.h"
#include "Systems/ClubhouseSystem.h"
#include "Data/GolfData.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

namespace
{
	UGameInstance* GetGI(const UWorld* world) { return world ? world->GetGameInstance() : nullptr; }
}

AGolfSystemTest::AGolfSystemTest()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGolfSystemTest::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Display, TEXT("[GolfTest] --- Full System Test ---"));

	RunCourseTests();
	RunStaffTests();
	RunDayCycleTests();
	RunClubhouseTests();

	UE_LOG(LogTemp, Display, TEXT("[GolfTest] --- Done: %d passed, %d failed ---"), passCount, failCount);
}

// ---------------------------------------------------------------- helpers

void AGolfSystemTest::ExpectTrue(const FString& label, bool condition)
{
	if (condition)
	{
		++passCount;
		UE_LOG(LogTemp, Display, TEXT("[GolfTest] PASS  %s"), *label);
	}
	else
	{
		++failCount;
		UE_LOG(LogTemp, Error, TEXT("[GolfTest] FAIL  %s"), *label);
	}
}

void AGolfSystemTest::ExpectInt(const FString& label, int32 expected, int32 actual)
{
	const FString text = FString::Printf(TEXT("%s (expect %d, got %d)"), *label, expected, actual);
	ExpectTrue(text, expected == actual);
}

void AGolfSystemTest::ExpectFloat(const FString& label, float expected, float actual)
{
	const FString text = FString::Printf(TEXT("%s (expect %.3f, got %.3f)"), *label, expected, actual);
	ExpectTrue(text, FMath::IsNearlyEqual(expected, actual, 0.001f));
}

void AGolfSystemTest::HandleMidnightPenalty()
{
	penaltySignalFired = true;
}

// ---------------------------------------------------------------- courses

void AGolfSystemTest::RunCourseTests()
{
	UGameInstance* gi = GetGI(GetWorld());
	UCourseManager* course = gi ? gi->GetSubsystem<UCourseManager>() : nullptr;
	if (!course)
	{
		ExpectTrue(TEXT("CourseManager exists"), false);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[GolfTest] -- Courses --"));

	const TArray<UHoleData*> holes = course->GetAllHoleData();
	ExpectInt(TEXT("Holes loaded"), 11, holes.Num());

	int32 parTotal = 0;
	for (const UHoleData* hole : holes)
	{
		parTotal += hole->par;
	}
	ExpectInt(TEXT("Par total"), 36, parTotal);

	ExpectTrue(TEXT("Unlock hole 4 at tier 0 fails"), !course->TryUnlockHole(4));
	ExpectInt(TEXT("Starting cash"), 1000, course->GetPlayerCash());
	ExpectTrue(TEXT("Unlock hole 2 succeeds"), course->TryUnlockHole(2));
	ExpectInt(TEXT("Cash after unlocking hole 2"), 850, course->GetPlayerCash());
}

// ---------------------------------------------------------------- staff

void AGolfSystemTest::RunStaffTests()
{
	UGameInstance* gi = GetGI(GetWorld());
	UCourseManager* course = gi ? gi->GetSubsystem<UCourseManager>() : nullptr;
	UStaffSystem* staff = gi ? gi->GetSubsystem<UStaffSystem>() : nullptr;
	if (!course || !staff || !jake || !maria)
	{
		ExpectTrue(TEXT("Staff test prerequisites (subsystems + Jake and Maria assigned on this actor)"), false);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[GolfTest] -- Staff --"));

	ExpectTrue(TEXT("Hire Jake"), staff->HireStaff(jake));
	ExpectInt(TEXT("Cash after hiring Jake"), 770, course->GetPlayerCash());
	ExpectTrue(TEXT("Hire Maria"), staff->HireStaff(maria));
	ExpectInt(TEXT("Cash after hiring Maria"), 570, course->GetPlayerCash());
	ExpectInt(TEXT("Roster size"), 2, staff->roster.Num());
	ExpectTrue(TEXT("Hiring Jake twice fails"), !staff->HireStaff(jake));

	ExpectTrue(TEXT("Assign Jake to hole 1"), staff->AssignToHole(jake, 1));
	ExpectTrue(TEXT("Assign Maria to hole 4 (locked) fails"), !staff->AssignToHole(maria, 4));
	ExpectTrue(TEXT("Assign Maria to hole 1"), staff->AssignToHole(maria, 1));

	ExpectInt(TEXT("Total daily wages"), 105, staff->GetTotalWages());
	ExpectFloat(TEXT("Hole 1 combined restore"), 0.30f, staff->GetMaintenanceForHole(1));
	const TMap<int32, float> effects = staff->GetMaintenanceEffects();
	const float* hole1Effect = effects.Find(1);
	ExpectFloat(TEXT("Hole 1 restore via effects map"), 0.30f, hole1Effect ? *hole1Effect : -1.0f);
}

// ---------------------------------------------------------------- day cycle

void AGolfSystemTest::RunDayCycleTests()
{
	UGameInstance* gi = GetGI(GetWorld());
	UCourseManager* course = gi ? gi->GetSubsystem<UCourseManager>() : nullptr;
	UStaffSystem* staff = gi ? gi->GetSubsystem<UStaffSystem>() : nullptr;
	UDayCycleSubsystem* day = gi ? gi->GetSubsystem<UDayCycleSubsystem>() : nullptr;
	if (!course || !staff || !day || !jake || !maria)
	{
		ExpectTrue(TEXT("Day cycle test prerequisites"), false);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[GolfTest] -- Day cycle --"));

	// Stop real-time ticking from interfering while we step manually.
	day->tickIntervalSeconds = 1000000.0f;

	ExpectTrue(TEXT("Starting phase is MorningPrep"), day->GetCurrentPhase() == EDayPhase::MorningPrep);
	ExpectFloat(TEXT("Starting time"), 300.0f, day->GetCurrentTimeMinutes());
	ExpectTrue(TEXT("Formatted start time"), day->GetFormattedTime() == TEXT("5:00 AM"));

	course->ApplyConditionChange(1, EConditionStat::Fairway, -0.5f);
	ExpectFloat(TEXT("Hole 1 fairway before day"), 0.50f, course->GetCondition(1, EConditionStat::Fairway));

	const int32 cashBefore = course->GetPlayerCash();

	// Step to 7:00 AM: 8 ticks of 15 minutes.
	for (int32 i = 0; i < 8; ++i)
	{
		day->AdvanceOneTick();
	}
	ExpectTrue(TEXT("Phase at 7:00 AM is OpenHours"), day->GetCurrentPhase() == EDayPhase::OpenHours);

	int32 safety = 0;
	while (day->GetCurrentPhase() != EDayPhase::Close && safety++ < 500)
	{
		day->AdvanceOneTick();
	}
	ExpectTrue(TEXT("Reached Close"), day->GetCurrentPhase() == EDayPhase::Close);
	ExpectTrue(TEXT("Formatted close time"), day->GetFormattedTime() == TEXT("7:00 PM"));

	// 0.50 + 0.30 maintenance - 0.05 decay
	ExpectFloat(TEXT("Hole 1 fairway after day"), 0.75f, course->GetCondition(1, EConditionStat::Fairway));
	ExpectInt(TEXT("Cash delta for the day"), 295, course->GetPlayerCash() - cashBefore);

	const FDayReport report = day->GetLastReport();
	ExpectInt(TEXT("Report day"), 1, report.day);
	ExpectInt(TEXT("Report revenue"), 400, report.revenue);
	ExpectInt(TEXT("Report wages"), 105, report.wages);
	ExpectInt(TEXT("Report net"), 295, report.net);

	const float frozenTime = day->GetCurrentTimeMinutes();
	day->AdvanceOneTick();
	ExpectFloat(TEXT("Clock frozen during Close"), frozenTime, day->GetCurrentTimeMinutes());

	ExpectTrue(TEXT("CloseOutDay succeeds"), day->CloseOutDay());
	ExpectInt(TEXT("Current day"), 2, day->GetCurrentDay());
	ExpectTrue(TEXT("Phase reset to MorningPrep"), day->GetCurrentPhase() == EDayPhase::MorningPrep);
	ExpectFloat(TEXT("Time reset"), 300.0f, day->GetCurrentTimeMinutes());
	ExpectFloat(TEXT("Close timer reset"), 0.0f, day->GetCloseElapsedMinutes());
	ExpectTrue(TEXT("CloseOutDay outside Close fails"), !day->CloseOutDay());

	// --- Midnight penalty: run day 2 to Close, then let the report go unclosed ---
	day->onMidnightPenaltyTriggered.AddDynamic(this, &AGolfSystemTest::HandleMidnightPenalty);

	safety = 0;
	while (day->GetCurrentPhase() != EDayPhase::Close && safety++ < 500)
	{
		day->AdvanceOneTick();
	}

	const int32 cashBeforePenalty = course->GetPlayerCash();
	const FStaffState* mariaState = staff->FindStaffState(maria);
	const float moraleBefore = mariaState ? mariaState->morale : -1.0f;

	safety = 0;
	while (day->GetCurrentDay() == 2 && safety++ < 500)
	{
		day->AdvanceOneTick();
	}

	ExpectTrue(TEXT("Midnight penalty signal fired"), penaltySignalFired);
	ExpectInt(TEXT("Cash change from penalty"), -100, course->GetPlayerCash() - cashBeforePenalty);
	ExpectFloat(TEXT("Maria morale before"), 1.0f, moraleBefore);
	mariaState = staff->FindStaffState(maria);
	ExpectFloat(TEXT("Maria morale after penalty"), 0.85f, mariaState ? mariaState->morale : -1.0f);
	ExpectInt(TEXT("Day after penalty auto-close"), 3, day->GetCurrentDay());

	day->onMidnightPenaltyTriggered.RemoveDynamic(this, &AGolfSystemTest::HandleMidnightPenalty);

	// --- Firing ---
	ExpectTrue(TEXT("Fire Jake"), staff->FireStaff(jake));
	ExpectInt(TEXT("Roster size after firing Jake"), 1, staff->roster.Num());
	ExpectInt(TEXT("Wages after firing"), 70, staff->GetTotalWages());
}

// ---------------------------------------------------------------- clubhouse

void AGolfSystemTest::RunClubhouseTests()
{
	UGameInstance* gi = GetGI(GetWorld());
	UCourseManager* course = gi ? gi->GetSubsystem<UCourseManager>() : nullptr;
	UClubhouseSystem* clubhouse = gi ? gi->GetSubsystem<UClubhouseSystem>() : nullptr;
	if (!course || !clubhouse)
	{
		ExpectTrue(TEXT("Clubhouse test prerequisites"), false);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[GolfTest] -- Clubhouse --"));

	// Substitute test rooms so this does not depend on authored assets.
	URoomData* kitchen = NewObject<URoomData>(this);
	kitchen->roomID = FName(TEXT("kitchen"));
	kitchen->tierRequired = 0;
	kitchen->buildCost = 200;

	URoomData* bar = NewObject<URoomData>(this);
	bar->roomID = FName(TEXT("bar"));
	bar->tierRequired = 1;
	bar->buildCost = 400;

	clubhouse->allRooms.Reset();
	clubhouse->allRooms.Add(kitchen);
	clubhouse->allRooms.Add(bar);

	const FName kitchenId(TEXT("kitchen"));
	const FName barId(TEXT("bar"));

	ExpectTrue(TEXT("Can unlock Kitchen at tier 0"), clubhouse->CanUnlock(kitchenId));
	ExpectTrue(TEXT("Cannot unlock Bar at tier 0"), !clubhouse->CanUnlock(barId));

	int32 cash = course->GetPlayerCash();
	ExpectTrue(TEXT("Build Kitchen"), clubhouse->BuildRoom(kitchenId));
	ExpectInt(TEXT("Cash change from Kitchen"), -200, course->GetPlayerCash() - cash);
	ExpectTrue(TEXT("Build Kitchen again fails"), !clubhouse->BuildRoom(kitchenId));
	ExpectTrue(TEXT("Build Bar before tier upgrade fails"), !clubhouse->BuildRoom(barId));

	cash = course->GetPlayerCash();
	ExpectTrue(TEXT("Upgrade clubhouse tier"), clubhouse->UpgradeClubhouseTier(300));
	ExpectInt(TEXT("Cash change from tier upgrade"), -300, course->GetPlayerCash() - cash);
	ExpectTrue(TEXT("Can unlock Bar after upgrade"), clubhouse->CanUnlock(barId));

	cash = course->GetPlayerCash();
	ExpectTrue(TEXT("Build Bar"), clubhouse->BuildRoom(barId));
	ExpectInt(TEXT("Cash change from Bar"), -400, course->GetPlayerCash() - cash);

	UFurnitureItemData* stool = NewObject<UFurnitureItemData>(this);
	stool->itemID = FName(TEXT("bar_stool"));
	stool->cost = 50;
	stool->roomTypes.Add(barId);
	stool->footprintSize = FIntPoint(1, 1);

	ExpectTrue(TEXT("Stool in Kitchen (wrong room) fails"), !clubhouse->PlaceItem(stool, kitchenId, FIntPoint(1, 1)));

	cash = course->GetPlayerCash();
	ExpectTrue(TEXT("Stool in Bar at (1,1)"), clubhouse->PlaceItem(stool, barId, FIntPoint(1, 1)));
	ExpectInt(TEXT("Cash change from stool"), -50, course->GetPlayerCash() - cash);
	ExpectTrue(TEXT("Second stool at same spot fails (conflict)"), !clubhouse->PlaceItem(stool, barId, FIntPoint(1, 1)));
	ExpectTrue(TEXT("Second stool at (2,1)"), clubhouse->PlaceItem(stool, barId, FIntPoint(2, 1)));

	ExpectInt(TEXT("Items in Bar"), 2, clubhouse->GetItemsInRoom(barId).Num());
	ExpectTrue(TEXT("Remove stool at (1,1)"), clubhouse->RemoveItem(barId, FIntPoint(1, 1)));
	ExpectTrue(TEXT("Remove same spot again fails"), !clubhouse->RemoveItem(barId, FIntPoint(1, 1)));
	ExpectInt(TEXT("Items in Bar after removal"), 1, clubhouse->GetItemsInRoom(barId).Num());
}
