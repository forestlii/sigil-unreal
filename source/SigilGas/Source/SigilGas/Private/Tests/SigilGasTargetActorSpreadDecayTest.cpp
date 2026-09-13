// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "TargetActors/SigilAbilityTargetActor_LineTrace.h"
#include "Tests/SigilGasTestWorld.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasTargetActorSpreadDecayTest,
	"SigilGas.TargetActor.SpreadDecaysOverTime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasTargetActorSpreadDecayTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasSpreadDecayWorld"));
	if (!Fixture.World)
	{
		AddError(TEXT("The test world could not be created"));
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASigilAbilityTargetActor_LineTrace* Target = Fixture.World->SpawnActor<ASigilAbilityTargetActor_LineTrace>(ASigilAbilityTargetActor_LineTrace::StaticClass(), FTransform::Identity, SpawnParameters);
	TestNotNull(TEXT("The line trace target actor should spawn"), Target);
	if (!Target)
	{
		return false;
	}

	Target->BaseSpread = 1.f;
	Target->TargetingSpreadIncrement = 2.f;
	Target->TargetingSpreadMax = 5.f;

	// --- Decay disabled (default): the original accumulate-and-clamp behaviour, unchanged by time.
	TestEqual(TEXT("Decay is off by default"), Target->TargetingSpreadDecayRate, 0.f);
	Target->AddTargetingSpread();
	Target->AddTargetingSpread();
	TestEqual(TEXT("Two shots accumulate two increments"), Target->GetCurrentTargetingSpread(), 4.f, KINDA_SMALL_NUMBER);
	Target->AddTargetingSpread();
	TestEqual(TEXT("Accumulation is clamped to the max"), Target->GetCurrentTargetingSpread(), 5.f, KINDA_SMALL_NUMBER);
	Fixture.World->TimeSeconds += 10.0;
	Target->UpdateTargetingSpreadDecay();
	TestEqual(TEXT("Without a decay rate time does not reduce the spread"), Target->GetCurrentTargetingSpread(), 5.f, KINDA_SMALL_NUMBER);
	TestEqual(TEXT("GetCurrentSpread adds the base spread"), Target->GetCurrentSpread(), 6.f, KINDA_SMALL_NUMBER);

	// --- Decay enabled with a delay.
	Target->ResetSpread();
	TestEqual(TEXT("ResetSpread clears the accumulated spread"), Target->GetCurrentTargetingSpread(), 0.f, KINDA_SMALL_NUMBER);
	Target->TargetingSpreadIncrement = 2.f;
	Target->TargetingSpreadMax = 5.f;
	Target->TargetingSpreadDecayRate = 4.f;
	Target->TargetingSpreadDecayDelay = 0.5f;

	Target->AddTargetingSpread();
	Target->AddTargetingSpread();
	TestEqual(TEXT("Spread after two shots"), Target->GetCurrentTargetingSpread(), 4.f, KINDA_SMALL_NUMBER);

	Fixture.World->TimeSeconds += 0.5;
	TestEqual(TEXT("No decay during the delay window"), Target->GetCurrentTargetingSpread(), 4.f, KINDA_SMALL_NUMBER);

	Fixture.World->TimeSeconds += 0.5;
	TestEqual(TEXT("Lazy read applies 0.5s of decay at 4 deg/s"), Target->GetCurrentTargetingSpread(), 2.f, KINDA_SMALL_NUMBER);

	// Settling into the stored value must not double count the same interval.
	Target->UpdateTargetingSpreadDecay();
	TestEqual(TEXT("Settling stores the decayed value"), Target->CurrentTargetingSpread, 2.f, KINDA_SMALL_NUMBER);
	Target->UpdateTargetingSpreadDecay();
	TestEqual(TEXT("Settling twice at the same time changes nothing"), Target->CurrentTargetingSpread, 2.f, KINDA_SMALL_NUMBER);

	// A new shot settles pending decay first, then adds its increment and restarts the delay.
	Fixture.World->TimeSeconds += 0.25;
	Target->AddTargetingSpread();
	TestEqual(TEXT("Shot after partial decay: 2 - 1 + 2"), Target->GetCurrentTargetingSpread(), 3.f, KINDA_SMALL_NUMBER);
	Fixture.World->TimeSeconds += 0.5;
	TestEqual(TEXT("The delay restarts after the shot"), Target->GetCurrentTargetingSpread(), 3.f, KINDA_SMALL_NUMBER);

	// Decay never goes below zero.
	Fixture.World->TimeSeconds += 5.0;
	TestEqual(TEXT("Decay clamps at zero"), Target->GetCurrentTargetingSpread(), 0.f, KINDA_SMALL_NUMBER);
	Target->UpdateTargetingSpreadDecay();
	TestEqual(TEXT("Settled value is zero"), Target->CurrentTargetingSpread, 0.f, KINDA_SMALL_NUMBER);

	return true;
}

#endif
