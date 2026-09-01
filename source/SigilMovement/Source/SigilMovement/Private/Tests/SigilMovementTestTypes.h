// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SigilCharacterMovementSystemComponent.h"
#include "SigilMovementTestTypes.generated.h"

class USigilMovementDefinition;

UCLASS(Transient)
class USigilMovementDeferredTestComponent final
	: public USigilCharacterMovementSystemComponent
{
	GENERATED_BODY()

public:
	void ConfigureStartupTest(
		const FGameplayTag& InMovementSet,
		TSoftObjectPtr<const USigilMovementDefinition> InDefinition);
};

UCLASS(Transient, NotPlaceable)
class ASigilMovementDeferredTestCharacter final : public ACharacter
{
	GENERATED_BODY()

public:
	ASigilMovementDeferredTestCharacter();

	USigilMovementDeferredTestComponent* GetMovementSystem() const
	{
		return MovementSystem;
	}

private:
	UPROPERTY()
	TObjectPtr<USigilMovementDeferredTestComponent> MovementSystem;
};
