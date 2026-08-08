// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Collision/GCS_AsyncAction_CollisionTrace.h"
#include "Collision/GCS_CollisionSystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Collision/GCS_CollisionTraceInstance.h"
#include "Engine/Engine.h"

UGCS_AsyncAction_CollisionTrace* UGCS_AsyncAction_CollisionTrace::SetupAndListenForCollisionTraceHit(UGCS_CollisionSystemComponent* CollisionSystem,
                                                                                                     TArray<FGCS_CollisionTraceDefinition> TraceDefinitions, UPrimitiveComponent* PrimitiveComponent)
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

	UGCS_AsyncAction_CollisionTrace* Action = NewObject<UGCS_AsyncAction_CollisionTrace>();

	Action->TraceDefinitions = TraceDefinitions;
	Action->CollisionSystemComponent = CollisionSystem;
	Action->PrimitiveComponent = PrimitiveComponent;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UGCS_AsyncAction_CollisionTrace::Activate()
{
	UGCS_CollisionSystemComponent* CSC = CollisionSystemComponent.Get();
	UPrimitiveComponent* Primitive = PrimitiveComponent.Get();

	if (CSC && Primitive)
	{
		CSC->OnTraceInstanceHitEvent.AddDynamic(this, &ThisClass::TraceInstanceHitCallback);

		TraceInstances = CSC->CreateTraceInstances(TraceDefinitions, Primitive);
		static FHitResult EmptyHitResult;
		for (UGCS_CollisionTraceInstance* TraceInstance : TraceInstances)
		{
			BeforeActive.Broadcast(TraceInstance, EmptyHitResult);
		}
		for (UGCS_CollisionTraceInstance* TraceInstance : TraceInstances)
		{
			TraceInstance->ToggleTraceState(true);
		}		
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UGCS_AsyncAction_CollisionTrace::Cancel()
{
	Super::Cancel();
	UGCS_CollisionSystemComponent* CSC = CollisionSystemComponent.Get();
	UPrimitiveComponent* Primitive = PrimitiveComponent.Get();
	if (CSC && Primitive)
	{
		CSC->OnTraceInstanceHitEvent.RemoveAll(this);
		//Deactivate traces.
		for (UGCS_CollisionTraceInstance* TraceInstance : TraceInstances)
		{
			TraceInstance->ToggleTraceState(false);
			CSC->RemoveTraceFromCreatedTraces(TraceInstance);
		}
		TraceInstances.Empty();
		PrimitiveComponent = nullptr;
		CollisionSystemComponent = nullptr;
	}
}

void UGCS_AsyncAction_CollisionTrace::TraceInstanceHitCallback(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	if (ShouldBroadcastDelegates())
	{
		if (TraceInstances.Contains(TraceInstance))
		{
			OnHit.Broadcast(TraceInstance, HitResult);
		}
	}
}
