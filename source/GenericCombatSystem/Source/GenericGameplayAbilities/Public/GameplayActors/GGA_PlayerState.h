// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GGA_AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "GGA_PlayerState.generated.h"

/**
 * @brief Minimal PlayerState class that supports extension by game feature plugins.
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API AGGA_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGGA_PlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	//~ End AActor interface

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	//~ Begin APlayerState interface
	virtual void CopyProperties(APlayerState* PlayerState);
	//~ End APlayerState interface

protected:
	/**
	 * Called by Controller when its PlayerState is initially replicated.
	 * @param Controller 
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveClientInitialize(AController* Controller);

private:
	UPROPERTY(Category=PlayerState, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UGGA_AbilitySystemComponent> AbilitySystemComponent;
};
