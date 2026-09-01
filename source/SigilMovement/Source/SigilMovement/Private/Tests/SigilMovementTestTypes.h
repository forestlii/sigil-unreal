// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SigilCharacterMovementSystemComponent.h"
#include "SigilMovementTestTypes.generated.h"

class USigilMovementDefinition;
class FDataValidationContext;

UCLASS(Transient)
class USigilMovementDeferredTestComponent final
	: public USigilCharacterMovementSystemComponent
{
	GENERATED_BODY()

public:
	void ConfigureStartupTest(
		const FGameplayTag& InMovementSet,
		TSoftObjectPtr<const USigilMovementDefinition> InDefinition);

	void ConfigureMovementStatesForTest(
		const TArray<FSigilMovementStateSetting>& InStates,
		FGameplayTag InDesiredState,
		float InSpeed);
	void SetLocomotionSpeedForTest(float InSpeed);
	void RefreshMovementStateForTest();
	UAnimInstance* GetMainAnimInstanceForTest() const;
	int32 GetMovementStateChangeCountForTest() const;

#if WITH_EDITORONLY_DATA
	EDataValidationResult ValidateForTest(FDataValidationContext& Context) const;
#endif

protected:
	virtual void OnMovementStateChanged_Implementation(
		const FGameplayTag& PreviousMovementState) override;

private:
	int32 MovementStateChangeCountForTest{0};
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
