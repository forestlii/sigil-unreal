// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "SigilAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "SigilPlayerState.generated.h"

/**
 * @brief Minimal PlayerState class that supports extension by game feature plugins.
 */
UCLASS()
class SIGILGAS_API ASigilPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASigilPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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
	TObjectPtr<USigilAbilitySystemComponent> AbilitySystemComponent;
};
