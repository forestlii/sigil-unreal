// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "InputCoreTypes.h"
#include "CommonUserWidget.h"
#include "GUIS_SimulatedInputWidget.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UCommonHardwareVisibilityBorder;
class UEnhancedPlayerInput;

/**
 *  A UMG widget with base functionality to inject input (keys or input actions)
 *  to the enhanced input subsystem.
 */
UCLASS()
class GENERICUISYSTEM_API UGUIS_SimulatedInputWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGUIS_SimulatedInputWidget(const FObjectInitializer& ObjectInitializer);

	//~ Begin UWidget
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	//~ End UWidget interface

	//~ Begin UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	//~ End UUserWidget interface

	/** Get the enhanced input subsystem based on the owning local player of this widget. Will return null if there is no owning player */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS")
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** Get the current player input from the current input subsystem */
	UEnhancedPlayerInput* GetPlayerInput() const;

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS")
	const UInputAction* GetAssociatedAction() const { return AssociatedAction; }

	/** Returns the current key that will be used to input any values. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS")
	FKey GetSimulatedKey() const { return KeyToSimulate; }

	/**
	 * Injects the given vector as an input to the current simulated key.
	 * This calls "InputKey" on the current player.
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	void InputKeyValue(const FVector& Value);

	/**
	 * Injects the given vector as an input to the current simulated key.
	 * This calls "InputKey" on the current player.
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	void InputKeyValue2D(const FVector2D& Value);

	UFUNCTION(BlueprintCallable, Category="GUIS")
	void FlushSimulatedInput();

protected:
	/** Set the KeyToSimulate based on a query from enhanced input about what keys are mapped to the associated action */
	void QueryKeyToSimulate();

	/** Called whenever control mappings change, so we have a chance to adapt our own keys */
	UFUNCTION()
	void OnControlMappingsRebuilt();

	/** The common visibility border will allow you to specify UI for only specific platforms if desired */
	UPROPERTY(BlueprintReadWrite, Category="GUIS", meta = (BindWidget))
	TObjectPtr<UCommonHardwareVisibilityBorder> CommonVisibilityBorder = nullptr;

	/** The associated input action that we should simulate input for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GUIS")
	TObjectPtr<const UInputAction> AssociatedAction = nullptr;

	/** The Key to simulate input for in the case where none are currently bound to the associated action */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GUIS")
	FKey FallbackBindingKey = EKeys::Gamepad_Right2D;

	/** The key that should be input via InputKey on the player input */
	FKey KeyToSimulate;
};
