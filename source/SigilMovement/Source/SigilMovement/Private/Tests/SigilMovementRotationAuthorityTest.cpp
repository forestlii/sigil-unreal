// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Locomotions/SigilMainAnimInstance.h"
#include "Locomotions/SigilSecondaryAnimInstance.h"
#include "Misc/ScopeExit.h"
#include "NativeGameplayTags.h"
#include "Settings/SigilSettingObjectLibrary.h"
#include "SigilCharacterMovementSystemComponent.h"
#include "Tests/SigilMovementTestTypes.h"
#include "UObject/Package.h"
#include "Utility/SigilMovementTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(
	SigilMovementRotationAuthorityTestSet,
	"Sigil.Movement.Set.RotationAuthorityAutomation");

namespace
{
struct FSigilMovementRotationTestWorld
{
	FSigilMovementRotationTestWorld()
	{
		if (!GEngine)
		{
			return;
		}

		const FName WorldName = MakeUniqueObjectName(
			nullptr,
			UWorld::StaticClass(),
			TEXT("SigilMovementRotationAuthorityTestWorld"),
			EUniqueObjectNameOptions::GloballyUnique);
		WorldContext = &GEngine->CreateNewWorldContext(EWorldType::Game);
		World = UWorld::CreateWorld(
			EWorldType::Game,
			false,
			WorldName,
			GetTransientPackage());
		if (World)
		{
			World->AddToRoot();
			WorldContext->SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}
	}

	~FSigilMovementRotationTestWorld()
	{
		if (!World || !GEngine)
		{
			return;
		}

		GEngine->ShutdownWorldNetDriver(World);
		World->DestroyWorld(true);
		World->SetPhysicsScene(nullptr);
		GEngine->DestroyWorldContext(World);
		World->RemoveFromRoot();
	}

	ASigilMovementDeferredTestCharacter* SpawnCharacter() const
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<ASigilMovementDeferredTestCharacter>(
			ASigilMovementDeferredTestCharacter::StaticClass(),
			FTransform::Identity,
			SpawnParameters);
	}

	UWorld* World = nullptr;
	FWorldContext* WorldContext = nullptr;
};

void SetAllRotationFlags(
	ASigilMovementDeferredTestCharacter& Character,
	const bool bEnabled)
{
	Character.bUseControllerRotationPitch = bEnabled;
	Character.bUseControllerRotationYaw = bEnabled;
	Character.bUseControllerRotationRoll = bEnabled;
	Character.GetCharacterMovement()->bOrientRotationToMovement = bEnabled;
	Character.GetCharacterMovement()->bUseControllerDesiredRotation = bEnabled;
}

void TestRotationFlags(
	FAutomationTestBase& Test,
	const TCHAR* Label,
	const ASigilMovementDeferredTestCharacter& Character,
	const bool bPitch,
	const bool bYaw,
	const bool bRoll,
	const bool bOrientToMovement,
	const bool bControllerDesired)
{
	Test.TestEqual(
		*FString::Printf(TEXT("%s Pawn Pitch"), Label),
		Character.bUseControllerRotationPitch,
		bPitch);
	Test.TestEqual(
		*FString::Printf(TEXT("%s Pawn Yaw"), Label),
		Character.bUseControllerRotationYaw,
		bYaw);
	Test.TestEqual(
		*FString::Printf(TEXT("%s Pawn Roll"), Label),
		Character.bUseControllerRotationRoll,
		bRoll);
	Test.TestEqual(
		*FString::Printf(TEXT("%s CMC OrientToMovement"), Label),
		Character.GetCharacterMovement()->bOrientRotationToMovement,
		bOrientToMovement);
	Test.TestEqual(
		*FString::Printf(TEXT("%s CMC ControllerDesired"), Label),
		Character.GetCharacterMovement()->bUseControllerDesiredRotation,
		bControllerDesired);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementRotationAuthorityTest,
	"SigilMovement.Runtime.RotationAuthority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementRotationAuthorityTest::RunTest(const FString& Parameters)
{
	const USigilCharacterMovementSystemComponent* DefaultComponent =
		GetDefault<USigilCharacterMovementSystemComponent>();
	if (!TestNotNull(TEXT("Default movement component should exist"), DefaultComponent))
	{
		return false;
	}
	TestEqual(
		TEXT("Default rotation authority should preserve SigilMovement ownership"),
		DefaultComponent->GetRotationAuthority(),
		ESigilMovementRotationAuthority::SigilMovement);

	FSigilMovementRotationTestWorld Fixture;
	ASigilMovementDeferredTestCharacter* Character = Fixture.SpawnCharacter();
	if (!TestNotNull(TEXT("Rotation authority character should spawn"), Character))
	{
		return false;
	}
	USigilCharacterMovementSystemComponent* Component = Character->GetMovementSystem();
	if (!TestNotNull(TEXT("Rotation authority component should exist"), Component))
	{
		return false;
	}

	SetAllRotationFlags(*Character, true);
	TestTrue(
		TEXT("SigilMovement authority should apply before configured runtime starts"),
		Component->SetRotationAuthority(
			ESigilMovementRotationAuthority::SigilMovement));
	TestRotationFlags(
		*this,
		TEXT("SigilMovement"),
		*Character,
		false,
		false,
		false,
		false,
		false);
	TestTrue(
		TEXT("Repeated SigilMovement authority application should be idempotent"),
		Component->SetRotationAuthority(
			ESigilMovementRotationAuthority::SigilMovement));
	TestRotationFlags(
		*this,
		TEXT("SigilMovement repeated"),
		*Character,
		false,
		false,
		false,
		false,
		false);

	TestTrue(
		TEXT("Controller authority should apply"),
		Component->SetRotationAuthority(
			ESigilMovementRotationAuthority::Controller));
	TestRotationFlags(
		*this,
		TEXT("Controller"),
		*Character,
		false,
		true,
		false,
		false,
		false);

	TestTrue(
		TEXT("MovementDirection authority should apply"),
		Component->SetRotationAuthority(
			ESigilMovementRotationAuthority::MovementDirection));
	TestRotationFlags(
		*this,
		TEXT("MovementDirection"),
		*Character,
		false,
		false,
		false,
		true,
		false);

	SetAllRotationFlags(*Character, true);
	TestTrue(
		TEXT("External authority should be selectable"),
		Component->SetRotationAuthority(
			ESigilMovementRotationAuthority::External));
	TestRotationFlags(
		*this,
		TEXT("External preserves flags"),
		*Character,
		true,
		true,
		true,
		true,
		true);

	TestFalse(
		TEXT("Invalid rotation authority should be rejected"),
		Component->SetRotationAuthority(
			static_cast<ESigilMovementRotationAuthority>(255)));
	TestEqual(
		TEXT("Rejected rotation authority must preserve the selected policy"),
		Component->GetRotationAuthority(),
		ESigilMovementRotationAuthority::External);
	TestRotationFlags(
		*this,
		TEXT("Rejected authority preserves flags"),
		*Character,
		true,
		true,
		true,
		true,
		true);

	if (!Character->HasActorBegunPlay())
	{
		Character->DispatchBeginPlay();
	}
	TestFalse(
		TEXT("Rotation authority must be locked after BeginPlay"),
		Component->SetRotationAuthority(
			ESigilMovementRotationAuthority::Controller));
	TestEqual(
		TEXT("BeginPlay lock must preserve the selected policy"),
		Component->GetRotationAuthority(),
		ESigilMovementRotationAuthority::External);
	TestRotationFlags(
		*this,
		TEXT("BeginPlay lock preserves flags"),
		*Character,
		true,
		true,
		true,
		true,
		true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementDeferredControlSettingsTest,
	"SigilMovement.Runtime.DeferredControlSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementDeferredControlSettingsTest::RunTest(
	const FString& Parameters)
{
	FSigilMovementRotationTestWorld Fixture;
	USigilMovementControlSetting_Default* ControlSetting =
		NewObject<USigilMovementControlSetting_Default>(GetTransientPackage());
	USigilMovementDefinition* Definition =
		NewObject<USigilMovementDefinition>(GetTransientPackage());
	if (!TestNotNull(TEXT("Control setting should be created"), ControlSetting)
		|| !TestNotNull(TEXT("Movement definition should be created"), Definition))
	{
		return false;
	}

	ControlSetting->bCanCrouch = true;
	FSigilMovementStateSetting Jog;
	Jog.Tag = SigilMovementStateTags::Jog;
	Jog.SpeedLevel = 0;
	Jog.Speed = 300.0f;
	Jog.CrouchedSpeed = 160.0f;
	FSigilMovementStateSetting Sprint;
	Sprint.Tag = SigilMovementStateTags::Sprint;
	Sprint.SpeedLevel = 1;
	Sprint.Speed = 520.0f;
	Sprint.CrouchedSpeed = 0.0f;
	ControlSetting->MovementStates = {Jog, Sprint};
	ControlSetting->TagToArrayIndex.Add(Jog.Tag, 0);
	ControlSetting->TagToArrayIndex.Add(Sprint.Tag, 1);
	ControlSetting->SpeedLevelToArrayIndex.Add(Jog.SpeedLevel, 0);
	ControlSetting->SpeedLevelToArrayIndex.Add(Sprint.SpeedLevel, 1);

	FSigilMovementSetSetting MovementSet;
	MovementSet.ControlSetting = ControlSetting;
	Definition->MovementSets.Add(
		SigilMovementRotationAuthorityTestSet.GetTag(),
		MovementSet);

	ASigilMovementDeferredTestCharacter* Character =
		Fixture.World->SpawnActorDeferred<ASigilMovementDeferredTestCharacter>(
			ASigilMovementDeferredTestCharacter::StaticClass(),
			FTransform::Identity);
	if (!TestNotNull(TEXT("Deferred control character should spawn"), Character))
	{
		return false;
	}
	USigilMovementDeferredTestComponent* Component = Character->GetMovementSystem();
	if (!TestNotNull(TEXT("Deferred test movement component should exist"), Component))
	{
		return false;
	}
	Component->ConfigureStartupTest(
		SigilMovementRotationAuthorityTestSet.GetTag(),
		TSoftObjectPtr<const USigilMovementDefinition>(Definition));
	Character->FinishSpawning(FTransform::Identity);
	if (!Character->HasActorBegunPlay())
	{
		Character->DispatchBeginPlay();
	}

	UCharacterMovementComponent* CharacterMovement =
		Character->GetCharacterMovement();
	if (!TestNotNull(TEXT("Character movement component should exist"), CharacterMovement))
	{
		return false;
	}
	TestFalse(
		TEXT("Control settings must not activate animation runtime"),
		Component->IsConfiguredRuntimeActive());
	TestEqual(
		TEXT("Deferred Jog should apply asset speed"),
		CharacterMovement->MaxWalkSpeed,
		300.0f);
	TestEqual(
		TEXT("Deferred Jog should apply asset crouched speed"),
		CharacterMovement->MaxWalkSpeedCrouched,
		160.0f);
	TestTrue(
		TEXT("Deferred control settings should enable crouch"),
		CharacterMovement->GetNavAgentPropertiesRef().bCanCrouch);

	Component->SetDesiredMovement(SigilMovementStateTags::Sprint);
	TestEqual(
		TEXT("Deferred Sprint should apply asset speed immediately"),
		CharacterMovement->MaxWalkSpeed,
		520.0f);
	TestEqual(
		TEXT("Deferred Sprint should apply an explicit zero crouched speed"),
		CharacterMovement->MaxWalkSpeedCrouched,
		0.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementActualMovementStateResolutionTest,
	"SigilMovement.Runtime.ActualMovementStateResolution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementActualMovementStateResolutionTest::RunTest(
	const FString& Parameters)
{
	FSigilMovementRotationTestWorld Fixture;
	ASigilMovementDeferredTestCharacter* Character = Fixture.SpawnCharacter();
	if (!TestNotNull(TEXT("Actual movement state character should spawn"), Character))
	{
		return false;
	}

	USigilMovementDeferredTestComponent* Component = Character->GetMovementSystem();
	if (!TestNotNull(TEXT("Actual movement state component should exist"), Component))
	{
		return false;
	}

	FSigilMovementStateSetting Walk;
	Walk.Tag = SigilMovementStateTags::Walk;
	Walk.SpeedLevel = 0;
	Walk.Speed = 150.0f;
	Walk.AllowedRotationModes = {SigilRotationModeTags::ViewDirection};
	FSigilMovementStateSetting Jog;
	Jog.Tag = SigilMovementStateTags::Jog;
	Jog.SpeedLevel = 1;
	Jog.Speed = 300.0f;
	Jog.AllowedRotationModes = {SigilRotationModeTags::ViewDirection};
	FSigilMovementStateSetting Sprint;
	Sprint.Tag = SigilMovementStateTags::Sprint;
	Sprint.SpeedLevel = 2;
	Sprint.Speed = 520.0f;
	Sprint.AllowedRotationModes = {SigilRotationModeTags::ViewDirection};
	const TArray<FSigilMovementStateSetting> States = {Walk, Jog, Sprint};

	Component->ConfigureMovementStatesForTest(
		States,
		SigilMovementStateTags::Jog,
		300.0f);
	TestTrue(
		TEXT("Initial desired movement state should be Jog"),
		Component->GetDesiredMovementState() == SigilMovementStateTags::Jog);
	TestTrue(
		TEXT("Initial actual movement state should be Jog"),
		Component->GetMovementState() == SigilMovementStateTags::Jog);

	Component->SetLocomotionSpeedForTest(118.0f);
	Component->RefreshMovementStateForTest();
	TestTrue(
		TEXT("Low actual speed should resolve Jog desire to Walk"),
		Component->GetMovementState() == SigilMovementStateTags::Walk);
	TestTrue(
		TEXT("Resolving actual movement state must preserve desired state"),
		Component->GetDesiredMovementState() == SigilMovementStateTags::Jog);
	const int32 StateChangesAfterWalk = Component->GetMovementStateChangeCountForTest();
	Component->RefreshMovementStateForTest();
	TestEqual(
		TEXT("Unchanged actual movement state must not broadcast twice"),
		Component->GetMovementStateChangeCountForTest(),
		StateChangesAfterWalk);

	Component->ConfigureMovementStatesForTest(
		States,
		SigilMovementStateTags::Sprint,
		510.0f);
	Component->RefreshMovementStateForTest();
	TestTrue(
		TEXT("Sprint desire should resolve to Sprint at sprint speed"),
		Component->GetMovementState() == SigilMovementStateTags::Sprint);
	Component->SetLocomotionSpeedForTest(250.0f);
	Component->RefreshMovementStateForTest();
	TestTrue(
		TEXT("Sprint desire should resolve to Jog after speed drops"),
		Component->GetMovementState() == SigilMovementStateTags::Jog);
	Component->SetLocomotionSpeedForTest(118.0f);
	Component->RefreshMovementStateForTest();
	TestTrue(
		TEXT("Sprint desire should resolve to Walk after speed drops further"),
		Component->GetMovementState() == SigilMovementStateTags::Walk);
	TestTrue(
		TEXT("Speed resolution must preserve Sprint desire"),
		Component->GetDesiredMovementState() == SigilMovementStateTags::Sprint);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilMovementSecondaryAnimInstanceTest,
	"SigilMovement.Runtime.SecondaryAnimInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilMovementSecondaryAnimInstanceTest::RunTest(const FString& Parameters)
{
	UClass* SecondaryClass = FindObject<UClass>(
		nullptr,
		TEXT("/Script/SigilMovement.SigilSecondaryAnimInstance"));
	if (!TestNotNull(TEXT("Secondary anim instance class should be registered"), SecondaryClass))
	{
		return false;
	}

	TestFalse(
		TEXT("Secondary anim instance must not inherit main anim instance behavior"),
		SecondaryClass->IsChildOf(USigilMainAnimInstance::StaticClass()));

	USkeletalMeshComponent* OwnerlessMesh = NewObject<USkeletalMeshComponent>(
		GetTransientPackage());
	if (!TestNotNull(TEXT("Ownerless mesh should be constructible"), OwnerlessMesh))
	{
		return false;
	}

	UAnimInstance* Secondary = NewObject<UAnimInstance>(
		OwnerlessMesh,
		SecondaryClass);
	if (!TestNotNull(TEXT("Secondary anim instance should be constructible without an owner"), Secondary))
	{
		return false;
	}

	Secondary->NativeInitializeAnimation();
	Secondary->NativeUpdateAnimation(1.0f / 60.0f);
	Secondary->NativeThreadSafeUpdateAnimation(1.0f / 60.0f);
	Secondary->NativeUninitializeAnimation();

	FSigilMovementRotationTestWorld Fixture;
	ASigilMovementDeferredTestCharacter* Character = Fixture.SpawnCharacter();
	if (!TestNotNull(TEXT("Secondary ownership character should spawn"), Character))
	{
		return false;
	}

	USigilMovementDeferredTestComponent* Component = Character->GetMovementSystem();
	if (!TestNotNull(TEXT("Secondary ownership component should exist"), Component))
	{
		return false;
	}
	Component->SetLocomotionSpeedForTest(118.0f);
	const UAnimInstance* MainBefore = Component->GetMainAnimInstanceForTest();
	USigilSecondaryAnimInstance* PawnSecondary =
		NewObject<USigilSecondaryAnimInstance>(Character->GetMesh());
	if (!TestNotNull(TEXT("Pawn secondary anim instance should be constructible"), PawnSecondary))
	{
		return false;
	}
	PawnSecondary->NativeInitializeAnimation();
	PawnSecondary->NativeUpdateAnimation(1.0f / 60.0f);
	TestTrue(
		TEXT("Secondary anim instance should copy the component movement state"),
		PawnSecondary->MovementState == Component->GetMovementState());
	TestTrue(
		TEXT("Secondary anim instance should copy the component locomotion mode"),
		PawnSecondary->LocomotionMode == Component->GetLocomotionMode());
	TestEqual(
		TEXT("Secondary anim instance should copy the component locomotion speed"),
		PawnSecondary->LocomotionState.Speed,
		118.0f);
	PawnSecondary->NativeUninitializeAnimation();
	TestNull(
		TEXT("Secondary anim instance lifecycle must not register a main anim instance"),
		Component->GetMainAnimInstanceForTest());
	TestTrue(
		TEXT("Secondary anim instance lifecycle must preserve the main anim instance pointer"),
		Component->GetMainAnimInstanceForTest() == MainBefore);
	return true;
}
#endif
