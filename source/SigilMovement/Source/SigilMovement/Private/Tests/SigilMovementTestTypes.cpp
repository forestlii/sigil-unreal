// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilMovementTestTypes.h"

#include "Animation/AnimInstance.h"
#include "Misc/DataValidation.h"
#include "Settings/SigilSettingObjectLibrary.h"
#include "Utility/SigilMovementTags.h"

void USigilMovementDeferredTestComponent::ConfigureStartupTest(
	const FGameplayTag& InMovementSet,
	TSoftObjectPtr<const USigilMovementDefinition> InDefinition)
{
	MovementSet = InMovementSet;
	MovementDefinitions = {InDefinition};
}

void USigilMovementDeferredTestComponent::ConfigureMovementStatesForTest(
	const TArray<FSigilMovementStateSetting>& InStates,
	const FGameplayTag InDesiredState,
	const float InSpeed)
{
	USigilMovementControlSetting_Default* TestControlSetting =
		NewObject<USigilMovementControlSetting_Default>(GetTransientPackage());
	TestControlSetting->MovementStates = InStates;
	for (int32 Index = 0; Index < InStates.Num(); ++Index)
	{
		TestControlSetting->TagToArrayIndex.Add(InStates[Index].Tag, Index);
		TestControlSetting->SpeedLevelToArrayIndex.Add(InStates[Index].SpeedLevel, Index);
	}

	ControlSetting = TestControlSetting;
	DesiredMovementState = InDesiredState;
	MovementState = InDesiredState;
	LocomotionState.Speed = InSpeed;
	MovementStateChangeCountForTest = 0;

	RefreshMovementStateSetting();
}

void USigilMovementDeferredTestComponent::SetLocomotionSpeedForTest(const float InSpeed)
{
	LocomotionState.Speed = InSpeed;
}

void USigilMovementDeferredTestComponent::RefreshMovementStateForTest()
{
	RefreshMovementState();
}

UAnimInstance* USigilMovementDeferredTestComponent::GetMainAnimInstanceForTest() const
{
	return MainAnimInstance.Get();
}

int32 USigilMovementDeferredTestComponent::GetMovementStateChangeCountForTest() const
{
	return MovementStateChangeCountForTest;
}

void USigilMovementDeferredTestComponent::OnMovementStateChanged_Implementation(
	const FGameplayTag& PreviousMovementState)
{
	++MovementStateChangeCountForTest;
	Super::OnMovementStateChanged_Implementation(PreviousMovementState);
}

#if WITH_EDITORONLY_DATA
EDataValidationResult USigilMovementDeferredTestComponent::ValidateForTest(
	FDataValidationContext& Context) const
{
	return IsDataValid(Context);
}
#endif

ASigilMovementDeferredTestCharacter::ASigilMovementDeferredTestCharacter()
{
	bUseControllerRotationYaw = true;
	MovementSystem = CreateDefaultSubobject<USigilMovementDeferredTestComponent>(TEXT("MovementSystem"));
	MovementSystem->SetRuntimeInitializationMode(
		ESigilMovementRuntimeInitializationMode::DeferredUntilConfigured);
}
