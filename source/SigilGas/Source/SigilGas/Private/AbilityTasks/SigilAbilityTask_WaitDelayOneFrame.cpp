// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AbilityTasks/SigilAbilityTask_WaitDelayOneFrame.h"
#include "Engine/World.h"
#include "TimerManager.h"

USigilAbilityTask_WaitDelayOneFrame::USigilAbilityTask_WaitDelayOneFrame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USigilAbilityTask_WaitDelayOneFrame::Activate()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &USigilAbilityTask_WaitDelayOneFrame::OnDelayFinish);
}

USigilAbilityTask_WaitDelayOneFrame* USigilAbilityTask_WaitDelayOneFrame::WaitDelayOneFrame(UGameplayAbility* OwningAbility)
{
	USigilAbilityTask_WaitDelayOneFrame* MyObj = NewAbilityTask<USigilAbilityTask_WaitDelayOneFrame>(OwningAbility);
	return MyObj;
}

void USigilAbilityTask_WaitDelayOneFrame::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}
	EndTask();
}
