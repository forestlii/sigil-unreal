// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SigilAbilityTask_WaitDelayOneFrame.generated.h"


/**
 * Like WaitDelay but only delays one frame (tick).
 */
UCLASS()
class SIGILGAS_API USigilAbilityTask_WaitDelayOneFrame : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitDelayOneFrameDelegate);
	UPROPERTY(BlueprintAssignable)
	FWaitDelayOneFrameDelegate OnFinish;

	virtual void Activate() override;

	// Like WaitDelay but only delays one frame (tick).
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USigilAbilityTask_WaitDelayOneFrame* WaitDelayOneFrame(UGameplayAbility* OwningAbility);

private:
	void OnDelayFinish();
};
