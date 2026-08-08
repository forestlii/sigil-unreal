// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "SigilAsyncTask_WaitGameplayAbilityActivated.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilAbilityActivatedDelegate, const UGameplayAbility*, Ability);

UCLASS()
class SIGILGAS_API USigilAsyncTask_WaitGameplayAbilityActivated : public UAbilityAsync
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static USigilAsyncTask_WaitGameplayAbilityActivated* WaitGameplayAbilityActivated(AActor* TargetActor);

	void HandleAbilityActivated(UGameplayAbility* Ability);

	UPROPERTY(BlueprintAssignable)
	FSigilAbilityActivatedDelegate OnAbilityActivated;

protected:
	virtual bool ShouldBroadcastDelegates() const override;
	virtual void Activate() override;
	virtual void EndAction() override;

	FDelegateHandle DelegateHandle;
};
