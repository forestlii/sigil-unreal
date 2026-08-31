// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilMovementTestTypes.h"

#include "SigilCharacterMovementSystemComponent.h"

ASigilMovementDeferredTestCharacter::ASigilMovementDeferredTestCharacter()
{
	bUseControllerRotationYaw = true;
	MovementSystem = CreateDefaultSubobject<USigilCharacterMovementSystemComponent>(TEXT("MovementSystem"));
	MovementSystem->SetRuntimeInitializationMode(
		ESigilMovementRuntimeInitializationMode::DeferredUntilConfigured);
}
