// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Async/GIS_AsyncAction_Wait.h"
#include "TimerManager.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_AsyncAction_Wait)


UGIS_AsyncAction_Wait::UGIS_AsyncAction_Wait()
{
}

bool UGIS_AsyncAction_Wait::ShouldBroadcastDelegates() const
{
	return Super::ShouldBroadcastDelegates() && IsValid(GetActor());
}

void UGIS_AsyncAction_Wait::StopWaiting()
{
	const UWorld* World = GetWorld();
	if (TimerHandle.IsValid() && IsValid(World))
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
}

void UGIS_AsyncAction_Wait::Cleanup()
{
	AActor* Actor = GetActor();

	if (IsValid(Actor))
	{
		Actor->OnDestroyed.RemoveDynamic(this, &ThisClass::OnTargetDestroyed);
	}

	StopWaiting();
}

void UGIS_AsyncAction_Wait::Activate()
{
	const UWorld* World = GetWorld();
	AActor* Actor = GetActor();
	if (IsValid(World) && IsValid(Actor))
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.SetTimer(TimerHandle, this, &ThisClass::OnTimer, WaitInterval, true, 0);
		Actor->OnDestroyed.AddDynamic(this, &ThisClass::OnTargetDestroyed);
	}
	else
	{
		Cancel();
	}
}

UWorld* UGIS_AsyncAction_Wait::GetWorld() const
{
	if (WorldPtr.IsValid() && WorldPtr->IsValidLowLevelFast())
	{
		return WorldPtr.Get();
	}

	return nullptr;
}

AActor* UGIS_AsyncAction_Wait::GetActor() const
{
	if (TargetActorPtr.IsValid() && TargetActorPtr->IsValidLowLevelFast())
	{
		return TargetActorPtr.Get();
	}

	return nullptr;
}

void UGIS_AsyncAction_Wait::Cancel()
{
	Super::Cancel();

	Cleanup();
}

void UGIS_AsyncAction_Wait::OnTargetDestroyed(AActor* DestroyedActor)
{
	Cancel();
}

void UGIS_AsyncAction_Wait::SetWorld(UWorld* NewWorld)
{
	WorldPtr = NewWorld;
}

void UGIS_AsyncAction_Wait::SetTargetActor(AActor* NewTargetActor)
{
	TargetActorPtr = NewTargetActor;
}

void UGIS_AsyncAction_Wait::SetWaitInterval(float NewWaitInterval)
{
	WaitInterval = NewWaitInterval;
}

void UGIS_AsyncAction_Wait::SetMaxWaitTimes(int32 NewMaxWaitTimes)
{
	MaxWaitTimes = NewMaxWaitTimes;
}

void UGIS_AsyncAction_Wait::OnTimer()
{
	AActor* Actor = GetActor();
	if (!IsValid(Actor))
	{
		Cancel();
		return;
	}

	OnExecutionAction();

	if (MaxWaitTimes > 0)
	{
		WaitTimes++;
		if (WaitTimes > MaxWaitTimes)
		{
			Cancel();
		}
	}
}

void UGIS_AsyncAction_Wait::OnExecutionAction()
{
}

void UGIS_AsyncAction_Wait::Complete()
{
	Super::Cancel();
	OnCompleted.Broadcast();
	Cleanup();
}
