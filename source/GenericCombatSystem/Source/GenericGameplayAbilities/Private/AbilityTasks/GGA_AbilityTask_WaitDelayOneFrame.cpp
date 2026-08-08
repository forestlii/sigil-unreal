// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AbilityTasks/GGA_AbilityTask_WaitDelayOneFrame.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGGA_AbilityTask_WaitDelayOneFrame::UGGA_AbilityTask_WaitDelayOneFrame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGGA_AbilityTask_WaitDelayOneFrame::Activate()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGGA_AbilityTask_WaitDelayOneFrame::OnDelayFinish);
}

UGGA_AbilityTask_WaitDelayOneFrame* UGGA_AbilityTask_WaitDelayOneFrame::WaitDelayOneFrame(UGameplayAbility* OwningAbility)
{
	UGGA_AbilityTask_WaitDelayOneFrame* MyObj = NewAbilityTask<UGGA_AbilityTask_WaitDelayOneFrame>(OwningAbility);
	return MyObj;
}

void UGGA_AbilityTask_WaitDelayOneFrame::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}
	EndTask();
}
