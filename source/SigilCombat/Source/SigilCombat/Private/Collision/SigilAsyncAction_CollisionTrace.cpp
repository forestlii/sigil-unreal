// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Collision/SigilAsyncAction_CollisionTrace.h"
#include "Collision/SigilCollisionSystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Collision/SigilCollisionTraceInstance.h"
#include "Engine/Engine.h"

USigilAsyncAction_CollisionTrace* USigilAsyncAction_CollisionTrace::SetupAndListenForCollisionTraceHit(USigilCollisionSystemComponent* CollisionSystem,
                                                                                                     TArray<FSigilCollisionTraceDefinition> TraceDefinitions, UPrimitiveComponent* PrimitiveComponent)
{
	if (CollisionSystem == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("SetupAndListenForCollisionTraceHit was passed a null CollisionSystem"), ELogVerbosity::Error);
		return nullptr;
	}

	if (PrimitiveComponent == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("SetupAndListenForCollisionTraceHit was passed a null PrimitiveComponent"), ELogVerbosity::Error);
		return nullptr;
	}

	if (TraceDefinitions.IsEmpty())
	{
		FFrame::KismetExecutionMessage(TEXT("SetupAndListenForCollisionTraceHit was passed empty TraceDefinitions"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(CollisionSystem, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	USigilAsyncAction_CollisionTrace* Action = NewObject<USigilAsyncAction_CollisionTrace>();

	Action->TraceDefinitions = TraceDefinitions;
	Action->CollisionSystemComponent = CollisionSystem;
	Action->PrimitiveComponent = PrimitiveComponent;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void USigilAsyncAction_CollisionTrace::Activate()
{
	USigilCollisionSystemComponent* CSC = CollisionSystemComponent.Get();
	UPrimitiveComponent* Primitive = PrimitiveComponent.Get();

	if (CSC && Primitive)
	{
		CSC->OnTraceInstanceHitEvent.AddDynamic(this, &ThisClass::TraceInstanceHitCallback);

		TraceInstances = CSC->CreateTraceInstances(TraceDefinitions, Primitive);
		static FHitResult EmptyHitResult;
		for (USigilCollisionTraceInstance* TraceInstance : TraceInstances)
		{
			BeforeActive.Broadcast(TraceInstance, EmptyHitResult);
		}
		for (USigilCollisionTraceInstance* TraceInstance : TraceInstances)
		{
			TraceInstance->ToggleTraceState(true);
		}		
	}
	else
	{
		SetReadyToDestroy();
	}
}

void USigilAsyncAction_CollisionTrace::Cancel()
{
	Super::Cancel();
	USigilCollisionSystemComponent* CSC = CollisionSystemComponent.Get();
	UPrimitiveComponent* Primitive = PrimitiveComponent.Get();
	if (CSC && Primitive)
	{
		CSC->OnTraceInstanceHitEvent.RemoveAll(this);
		//Deactivate traces.
		for (USigilCollisionTraceInstance* TraceInstance : TraceInstances)
		{
			TraceInstance->ToggleTraceState(false);
			CSC->RemoveTraceFromCreatedTraces(TraceInstance);
		}
		TraceInstances.Empty();
		PrimitiveComponent = nullptr;
		CollisionSystemComponent = nullptr;
	}
}

void USigilAsyncAction_CollisionTrace::TraceInstanceHitCallback(USigilCollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	if (ShouldBroadcastDelegates())
	{
		if (TraceInstances.Contains(TraceInstance))
		{
			OnHit.Broadcast(TraceInstance, HitResult);
		}
	}
}
