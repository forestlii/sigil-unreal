// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilMovementTestTypes.h"

#include "Misc/DataValidation.h"
#include "Settings/SigilSettingObjectLibrary.h"

void USigilMovementDeferredTestComponent::ConfigureStartupTest(
	const FGameplayTag& InMovementSet,
	TSoftObjectPtr<const USigilMovementDefinition> InDefinition)
{
	MovementSet = InMovementSet;
	MovementDefinitions = {InDefinition};
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
