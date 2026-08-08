// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GGA_AbilityTask_WaitDelayOneFrame.generated.h"


/**
 * Like WaitDelay but only delays one frame (tick).
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_AbilityTask_WaitDelayOneFrame : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitDelayOneFrameDelegate);
	UPROPERTY(BlueprintAssignable)
	FWaitDelayOneFrameDelegate OnFinish;

	virtual void Activate() override;

	// Like WaitDelay but only delays one frame (tick).
	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGGA_AbilityTask_WaitDelayOneFrame* WaitDelayOneFrame(UGameplayAbility* OwningAbility);

private:
	void OnDelayFinish();
};
