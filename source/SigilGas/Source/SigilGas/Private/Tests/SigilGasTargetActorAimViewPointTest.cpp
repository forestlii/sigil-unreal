// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "TargetActors/SigilAbilityTargetActor_LineTrace.h"
#include "Tests/SigilGasTestTypes.h"
#include "Tests/SigilGasTestWorld.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilGasTargetActorAimViewPointTest,
	"SigilGas.TargetActor.AimViewPointRespectsFlags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilGasTargetActorAimViewPointTest::RunTest(const FString& Parameters)
{
	FSigilGasTestWorld Fixture(TEXT("SigilGasAimViewPointWorld"));
	if (!Fixture.World)
	{
		AddError(TEXT("The test world could not be created"));
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASigilAbilityTargetActor_LineTrace* Target = Fixture.World->SpawnActor<ASigilAbilityTargetActor_LineTrace>(ASigilAbilityTargetActor_LineTrace::StaticClass(), FTransform::Identity, SpawnParameters);
	ASigilGasTestPlayerController* PC = Fixture.World->SpawnActor<ASigilGasTestPlayerController>(ASigilGasTestPlayerController::StaticClass(), FTransform::Identity, SpawnParameters);
	TestNotNull(TEXT("The target actor should spawn"), Target);
	TestNotNull(TEXT("The player controller should spawn"), PC);
	if (!Target || !PC)
	{
		return false;
	}

	// The controller looks along +Y; the start location looks along +X from a different spot.
	const FRotator ControllerRotation(0.f, 90.f, 0.f);
	const FVector ControllerLocation(0.f, 0.f, 500.f);
	PC->ScriptedViewLocation = ControllerLocation;
	PC->ScriptedViewRotation = ControllerRotation;

	FVector PCViewStart;
	FRotator PCViewRot;
	PC->GetPlayerViewPoint(PCViewStart, PCViewRot);
	TestEqual(TEXT("Sanity: the controller's view yaw is what the test set"), PCViewRot.Yaw, 90.0, 0.01);

	FGameplayAbilityTargetingLocationInfo StartLocation;
	StartLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	StartLocation.LiteralTransform = FTransform(FRotator::ZeroRotator, FVector(100.f, 0.f, 0.f));
	Target->SetStartLocation(StartLocation);

	const FVector TraceStart(100.f, 0.f, 0.f);
	FVector ViewStart;
	FRotator ViewRot;

	// No controller: always StartLocation, whatever the flags say.
	Target->PrimaryPC = nullptr;
	Target->bAlwaysAimWithPlayerController = true;
	Target->bTraceFromPlayerViewPoint = true;
	Target->GetAimViewPoint(TraceStart, ViewStart, ViewRot);
	TestEqual(TEXT("Without a controller the aim view starts at TraceStart"), ViewStart, TraceStart);
	TestEqual(TEXT("Without a controller the aim yaw is StartLocation's"), ViewRot.Yaw, 0.0, 0.01);

	// Controller present, both flags off (Sigil default): StartLocation rotation, not the camera.
	Target->PrimaryPC = PC;
	Target->bAlwaysAimWithPlayerController = false;
	Target->bTraceFromPlayerViewPoint = false;
	Target->GetAimViewPoint(TraceStart, ViewStart, ViewRot);
	TestEqual(TEXT("Default flags aim along StartLocation even with a controller"), ViewRot.Yaw, 0.0, 0.01);
	TestEqual(TEXT("Default flags keep the aim view at TraceStart"), ViewStart, TraceStart);

	// bTraceFromPlayerViewPoint: the camera view (existing behaviour).
	Target->bTraceFromPlayerViewPoint = true;
	Target->GetAimViewPoint(TraceStart, ViewStart, ViewRot);
	TestEqual(TEXT("bTraceFromPlayerViewPoint aims along the controller view"), ViewRot.Yaw, 90.0, 0.01);
	TestEqual(TEXT("bTraceFromPlayerViewPoint starts the aim view at the controller view point"), ViewStart, PCViewStart);

	// bAlwaysAimWithPlayerController alone: the camera view (GASShooter behaviour) while the trace still starts elsewhere.
	Target->bTraceFromPlayerViewPoint = false;
	Target->bAlwaysAimWithPlayerController = true;
	Target->GetAimViewPoint(TraceStart, ViewStart, ViewRot);
	TestEqual(TEXT("bAlwaysAimWithPlayerController aims along the controller view"), ViewRot.Yaw, 90.0, 0.01);
	TestEqual(TEXT("bAlwaysAimWithPlayerController starts the aim view at the controller view point"), ViewStart, PCViewStart);

	return true;
}

#endif
