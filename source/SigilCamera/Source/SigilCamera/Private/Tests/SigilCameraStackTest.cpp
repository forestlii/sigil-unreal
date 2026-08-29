// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "SigilCameraModeStack.h"
#include "Tests/SigilCameraStackTestTypes.h"
#include "UObject/Package.h"

namespace
{
TSubclassOf<USigilCameraMode> SelectStackTestMode()
{
	return USigilCameraStackTestMode::StaticClass();
}

UWorld* CreateCameraTestWorld()
{
	return UWorld::CreateWorld(EWorldType::Game, false);
}

AActor* CreateCameraTestOwner(UWorld* World)
{
	return World ? NewObject<AActor>(World->PersistentLevel) : nullptr;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilCameraEmptyStackEvaluateTest,
	"SigilCamera.Stack.Empty.EvaluateReturnsFalseAndPreservesView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilCameraEmptyStackComponentTest,
	"SigilCamera.Stack.Empty.ComponentTickPreservesAssociatedView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilCameraStackAppliesViewTest,
	"SigilCamera.Stack.Effective.ComponentTickAppliesViewAndConsumesOffset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilCameraEmptyStackEvaluateTest::RunTest(const FString& Parameters)
{
	USigilCameraModeStack* Stack = NewObject<USigilCameraModeStack>(GetTransientPackage());
	FSigilCameraModeView View;
	View.Location = FVector(1.0f, 2.0f, 3.0f);
	View.Rotation = FRotator(4.0f, 5.0f, 6.0f);
	View.ControlRotation = FRotator(7.0f, 8.0f, 9.0f);
	View.FieldOfView = 81.0f;
	View.SpringArmLength = 456.0f;
	View.SpringArmSocketOffset = FVector(10.0f, 11.0f, 12.0f);
	View.SpringArmTargetOffset = FVector(13.0f, 14.0f, 15.0f);

	if (!TestNotNull(TEXT("Empty camera stack should exist"), Stack))
	{
		return false;
	}

	TestFalse(TEXT("An active empty stack should not evaluate a camera view"), Stack->EvaluateStack(0.016f, View));
	TestTrue(TEXT("An empty stack should preserve location"), View.Location.Equals(FVector(1.0f, 2.0f, 3.0f)));
	TestTrue(TEXT("An empty stack should preserve rotation"), View.Rotation.Equals(FRotator(4.0f, 5.0f, 6.0f)));
	TestTrue(TEXT("An empty stack should preserve control rotation"), View.ControlRotation.Equals(FRotator(7.0f, 8.0f, 9.0f)));
	TestEqual(TEXT("An empty stack should preserve field of view"), View.FieldOfView, 81.0f);
	TestEqual(TEXT("An empty stack should preserve spring arm length"), View.SpringArmLength, 456.0f);
	TestTrue(TEXT("An empty stack should preserve socket offset"), View.SpringArmSocketOffset.Equals(FVector(10.0f, 11.0f, 12.0f)));
	TestTrue(TEXT("An empty stack should preserve target offset"), View.SpringArmTargetOffset.Equals(FVector(13.0f, 14.0f, 15.0f)));
	return true;
}

bool FSigilCameraEmptyStackComponentTest::RunTest(const FString& Parameters)
{
	UWorld* World = CreateCameraTestWorld();
	AActor* Owner = CreateCameraTestOwner(World);
	USigilCameraStackTestComponent* Component = NewObject<USigilCameraStackTestComponent>(Owner);
	UCameraComponent* Camera = NewObject<UCameraComponent>(Component);
	USpringArmComponent* SpringArm = NewObject<USpringArmComponent>(Component);

	if (!TestNotNull(TEXT("Camera system test world should exist"), World)
		|| !TestNotNull(TEXT("Camera system owner should exist"), Owner)
		|| !TestNotNull(TEXT("Camera system component should exist"), Component)
		|| !TestNotNull(TEXT("Associated camera should exist"), Camera)
		|| !TestNotNull(TEXT("Associated spring arm should exist"), SpringArm))
	{
		return false;
	}

	Camera->FieldOfView = 71.0f;
	SpringArm->TargetArmLength = 555.0f;
	SpringArm->SocketOffset = FVector(31.0f, 32.0f, 33.0f);
	SpringArm->TargetOffset = FVector(41.0f, 42.0f, 43.0f);
	Owner->AddInstanceComponent(Component);
	Component->RegisterComponentWithWorld(World);
	Component->Initialize(Camera, SpringArm);
	Component->AddFieldOfViewOffset(12.0f);
	Component->TickForTest(0.016f);

	TestEqual(TEXT("An empty stack tick should preserve camera field of view"), Camera->FieldOfView, 71.0f);
	TestEqual(TEXT("An empty stack tick should preserve spring arm length"), SpringArm->TargetArmLength, 555.0f);
	TestTrue(TEXT("An empty stack tick should preserve socket offset"), SpringArm->SocketOffset.Equals(FVector(31.0f, 32.0f, 33.0f)));
	TestTrue(TEXT("An empty stack tick should preserve target offset"), SpringArm->TargetOffset.Equals(FVector(41.0f, 42.0f, 43.0f)));
	TestEqual(TEXT("An empty stack tick should retain the unapplied field of view offset"), Component->GetFieldOfViewOffsetForTest(), 12.0f);
	return true;
}

bool FSigilCameraStackAppliesViewTest::RunTest(const FString& Parameters)
{
	UWorld* World = CreateCameraTestWorld();
	AActor* Owner = CreateCameraTestOwner(World);
	USigilCameraStackTestComponent* Component = NewObject<USigilCameraStackTestComponent>(Owner);
	UCameraComponent* Camera = NewObject<UCameraComponent>(Component);
	USpringArmComponent* SpringArm = NewObject<USpringArmComponent>(Component);

	if (!TestNotNull(TEXT("Camera system test world should exist"), World)
		|| !TestNotNull(TEXT("Camera system owner should exist"), Owner)
		|| !TestNotNull(TEXT("Camera system component should exist"), Component)
		|| !TestNotNull(TEXT("Associated camera should exist"), Camera)
		|| !TestNotNull(TEXT("Associated spring arm should exist"), SpringArm))
	{
		return false;
	}

	Owner->AddInstanceComponent(Component);
	Component->RegisterComponentWithWorld(World);
	Component->Initialize(Camera, SpringArm);
	Component->DetermineCameraModeDelegate.BindStatic(&SelectStackTestMode);
	Component->AddFieldOfViewOffset(12.0f);
	Component->TickForTest(0.016f);

	TestEqual(TEXT("An evaluated view should apply the field of view offset once"), Camera->FieldOfView, 102.0f);
	TestEqual(TEXT("An evaluated view should apply spring arm length"), SpringArm->TargetArmLength, 420.0f);
	TestTrue(TEXT("An evaluated view should apply socket offset"), SpringArm->SocketOffset.Equals(FVector(11.0f, 12.0f, 13.0f)));
	TestTrue(TEXT("An evaluated view should apply target offset"), SpringArm->TargetOffset.Equals(FVector(21.0f, 22.0f, 23.0f)));
	TestEqual(TEXT("An evaluated view should consume the applied field of view offset"), Component->GetFieldOfViewOffsetForTest(), 0.0f);

	Component->TickForTest(0.016f);
	TestEqual(TEXT("A consumed one-frame offset should not apply twice"), Camera->FieldOfView, 90.0f);
	return true;
}

#endif
