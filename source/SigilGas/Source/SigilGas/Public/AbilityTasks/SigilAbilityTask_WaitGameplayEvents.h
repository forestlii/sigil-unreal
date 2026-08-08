// Copyright (c) 2026 Likeon. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SigilAbilityTask_WaitGameplayEvents.generated.h"

class UAbilitySystemComponent;


UCLASS()
class SIGILGAS_API USigilAbilityTask_WaitGameplayEvents : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitGameplayEventsDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

	UPROPERTY(BlueprintAssignable)
	FWaitGameplayEventsDelegate EventReceived;

	/**
	 * Wait until the specified gameplay tags event is triggered. By default this will look at the owner of this ability. OptionalExternalTarget can be set to make this look at another actor's tags for changes
	 * It will keep listening as long as OnlyTriggerOnce = false
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USigilAbilityTask_WaitGameplayEvents* WaitGameplayEvents(UGameplayAbility* OwningAbility, FGameplayTagContainer EventTags, AActor* OptionalExternalTarget = nullptr,
	                                                               bool OnlyTriggerOnce = false);

	void SetExternalTarget(AActor* Actor);

	UAbilitySystemComponent* GetTargetASC();

	virtual void Activate() override;

	virtual void GameplayEventContainerCallback(FGameplayTag MatchingTag, const FGameplayEventData* Payload);

	void OnDestroy(bool AbilityEnding) override;

	/** List of tags to match against gameplay events */
	UPROPERTY()
	FGameplayTagContainer EventTags;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OptionalExternalTarget;

	bool UseExternalTarget;
	bool OnlyTriggerOnce;

	FDelegateHandle MyHandle;
};
