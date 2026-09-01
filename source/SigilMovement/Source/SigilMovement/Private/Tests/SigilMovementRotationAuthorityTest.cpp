// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
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
#endif
