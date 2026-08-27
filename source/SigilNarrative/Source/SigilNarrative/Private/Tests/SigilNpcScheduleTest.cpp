// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "SigilNpcScheduleAsset.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilNarrativeNpcScheduleResolvesDailyLocationsTest,
	"SigilNarrative.NpcSchedule.ResolvesDailyLocations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilNarrativeNpcScheduleResolvesDailyLocationsTest::RunTest(
	const FString& Parameters)
{
	USigilNpcScheduleAsset* Schedule =
		NewObject<USigilNpcScheduleAsset>(GetTransientPackage());
	TestNotNull(TEXT("NPC schedule should be constructible"), Schedule);
	if (!Schedule)
	{
		return false;
	}

	FSigilNpcScheduleEntry Work;
	Work.StartMinute = 8 * 60;
	Work.ActivityId = TEXT("Activity.Work");
	Work.LocationId = TEXT("Location.Work");

	FSigilNpcScheduleEntry Home;
	Home.StartMinute = 18 * 60;
	Home.ActivityId = TEXT("Activity.Home");
	Home.LocationId = TEXT("Location.Home");

	Schedule->Entries = { Work, Home };

	FSigilNpcScheduleEntry Resolved;
	TestTrue(
		TEXT("07:00 should wrap to the previous day's home entry"),
		Schedule->ResolveAtMinute(7 * 60, Resolved));
	TestEqual(
		TEXT("07:00 should resolve home"),
		Resolved.LocationId,
		FName(TEXT("Location.Home")));

	TestTrue(
		TEXT("08:00 should resolve the work entry"),
		Schedule->ResolveAtMinute(8 * 60, Resolved));
	TestEqual(
		TEXT("08:00 should resolve work"),
		Resolved.LocationId,
		FName(TEXT("Location.Work")));

	TestTrue(
		TEXT("18:00 should resolve the home entry"),
		Schedule->ResolveAtMinute(18 * 60, Resolved));
	TestEqual(
		TEXT("18:00 should resolve home"),
		Resolved.LocationId,
		FName(TEXT("Location.Home")));

	TestFalse(
		TEXT("Minutes outside a day should be rejected"),
		Schedule->ResolveAtMinute(24 * 60, Resolved));

	return true;
}

#endif
