// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilMovementTestTypes.h"

#include "Settings/SigilSettingObjectLibrary.h"

void USigilMovementDeferredTestComponent::ConfigureStartupTest(
	const FGameplayTag& InMovementSet,
	TSoftObjectPtr<const USigilMovementDefinition> InDefinition)
{
	MovementSet = InMovementSet;
	MovementDefinitions = {InDefinition};
}

ASigilMovementDeferredTestCharacter::ASigilMovementDeferredTestCharacter()
{
	bUseControllerRotationYaw = true;
	MovementSystem = CreateDefaultSubobject<USigilMovementDeferredTestComponent>(TEXT("MovementSystem"));
	MovementSystem->SetRuntimeInitializationMode(
		ESigilMovementRuntimeInitializationMode::DeferredUntilConfigured);
}
