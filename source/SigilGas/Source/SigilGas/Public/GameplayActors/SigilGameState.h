// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameStateBase.h"
#include "SigilGameState.generated.h"

class USigilAbilitySystemComponent;
class UObject;


/**
 * @brief A Game State base with ability system component.
 */
UCLASS(Blueprintable)
class SIGILGAS_API ASigilGameStateBase : public AGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ASigilGameStateBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

private:
	// The ability system component subobject for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "GGA|GameState")
	TObjectPtr<USigilAbilitySystemComponent> AbilitySystemComponent;
};


/**
 * @brief A Game State with ability system component.
 */
UCLASS(Blueprintable)
class SIGILGAS_API ASigilGameState : public AGameState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ASigilGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

	protected:
	//~ Begin AGameState interface
	virtual void HandleMatchHasStarted() override;
	//~ Begin AGameState interface


	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

private:
	// The ability system component subobject for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "GameState")
	TObjectPtr<USigilAbilitySystemComponent> AbilitySystemComponent;
};
