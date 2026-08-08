// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameStateBase.h"
#include "GGA_GameState.generated.h"

class UGGA_AbilitySystemComponent;
class UObject;


/**
 * @brief A Game State base with ability system component.
 */
UCLASS(Blueprintable)
class GENERICGAMEPLAYABILITIES_API AGGA_GameStateBase : public AGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AGGA_GameStateBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
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
	TObjectPtr<UGGA_AbilitySystemComponent> AbilitySystemComponent;
};


/**
 * @brief A Game State with ability system component.
 */
UCLASS(Blueprintable)
class GENERICGAMEPLAYABILITIES_API AGGA_GameState : public AGameState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AGGA_GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
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
	TObjectPtr<UGGA_AbilitySystemComponent> AbilitySystemComponent;
};
