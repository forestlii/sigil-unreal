// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SigilMovementTestTypes.generated.h"

class USigilCharacterMovementSystemComponent;

UCLASS(Transient, NotPlaceable)
class ASigilMovementDeferredTestCharacter final : public ACharacter
{
	GENERATED_BODY()

public:
	ASigilMovementDeferredTestCharacter();

	USigilCharacterMovementSystemComponent* GetMovementSystem() const
	{
		return MovementSystem;
	}

private:
	UPROPERTY()
	TObjectPtr<USigilCharacterMovementSystemComponent> MovementSystem;
};
