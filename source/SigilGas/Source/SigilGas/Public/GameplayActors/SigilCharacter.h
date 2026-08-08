// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "SigilCharacter.generated.h"


/**
 * @brief A character with ability system.
 */
UCLASS()
class SIGILGAS_API ASigilCharacter : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface

{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASigilCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Pawn

protected:
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	/**
	 * @brief Called when controller replicated to client.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Character, meta=(DisplayName="Receive Player Controller"))
	void ReceivePlayerController();

	/**
	 * @brief Called when player state replicated to client.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Character, meta=(DisplayName="Receive Player State"))
	void ReceivePlayerState();

#pragma endregion Pawn


#pragma region AbilitySystem

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	UAbilitySystemComponent* CustomGetAbilitySystemComponent() const;

private:
#pragma endregion AbilitySystem

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
