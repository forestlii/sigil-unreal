// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Collision/GCS_CollisionSystemComponent.h"
#include "GCS_LogChannels.h"
#include "Collision/GCS_CollisionTraceInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "TargetingSystem/TargetingPreset.h"
#include "Utility/GCS_CombatFunctionLibrary.h"

UGCS_CollisionSystemComponent::UGCS_CollisionSystemComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

UGCS_CollisionSystemComponent* UGCS_CollisionSystemComponent::GetCollisionSystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<UGCS_CollisionSystemComponent>() : nullptr;
}

bool UGCS_CollisionSystemComponent::FindCollisionSystemComponent(const AActor* Actor, UGCS_CollisionSystemComponent*& Component)
{
	Component = GetCollisionSystemComponent(Actor);
	return Component != nullptr;
}

void UGCS_CollisionSystemComponent::OnInitialize_Implementation()
{
	CreateDefaultTraceInstances();
}

void UGCS_CollisionSystemComponent::OnDeinitialize_Implementation()
{
	ClearTraceInstances();
}

// Called when the game starts
void UGCS_CollisionSystemComponent::BeginPlay()
{
	if (bAutoInitialize)
	{
		OnInitialize();
	}
	Super::BeginPlay();
}

void UGCS_CollisionSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bAutoInitialize)
	{
		OnDeinitialize();
	}
	Super::EndPlay(EndPlayReason);
}


UGCS_CollisionTraceInstance* UGCS_CollisionSystemComponent::GetOrCreateTraceInstance(TSubclassOf<UGCS_CollisionTraceInstance> TraceClass)
{
	UGCS_CollisionTraceInstance* RetInstance = nullptr;
	if (IsValid(TraceClass))
	{
		FGCS_TraceInstancesCache& Cache = CachedTraceInstances.FindOrAdd(TraceClass);
		if (Cache.Instances.IsEmpty())
		{
			RetInstance = NewObject<UGCS_CollisionTraceInstance>(GetOwner(), TraceClass);
			RetInstance->TraceOwner = GetOwner();
		}
		else
		{
			RetInstance = Cache.Instances.Pop();
		}
		if (RetInstance)
		{
			TraceInstances.AddUnique(RetInstance);
			RetInstance->OnTraceBeginPlay();
		}
	}
	return RetInstance;
}

// Called every frame
void UGCS_CollisionSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickActiveTraces(DeltaTime);
}

TArray<UGCS_CollisionTraceInstance*> UGCS_CollisionSystemComponent::CreateTraceInstances(TArray<FGCS_CollisionTraceDefinition> Definitions, UPrimitiveComponent* PrimitiveComponent)
{
	TArray<UGCS_CollisionTraceInstance*> Ret;
	if (PrimitiveComponent)
	{
		for (const FGCS_CollisionTraceDefinition& Def : Definitions)
		{
			if (Def.TraceClass.IsNull() || !Def.TraceTag.IsValid())
				continue;

			TArray<FName> PrimitiveSocketNames = Def.SocketNames.IsEmpty()
				                                     ? UGCS_CombatFunctionLibrary::GetSocketNamesWithPrefix(PrimitiveComponent, Def.SocketPrefix.ToString(), ESearchCase::IgnoreCase)
				                                     : Def.SocketNames;

			if (TSubclassOf<UGCS_CollisionTraceInstance> TraceClass = Def.TraceClass.LoadSynchronous())
			{
				if (UGCS_CollisionTraceInstance* NewTraceInstance = GetOrCreateTraceInstance(TraceClass))
				{
					NewTraceInstance->TraceGameplayTag = Def.TraceTag;
					NewTraceInstance->TracePrimitiveComponent = PrimitiveComponent;
					NewTraceInstance->TracePrimitiveComponentSocketNames = PrimitiveSocketNames;
					if (!Def.TargetingPreset.IsNull())
					{
						if (UTargetingPreset* TS = Def.TargetingPreset.LoadSynchronous())
						{
							NewTraceInstance->TargetingPreset = TS;
						}
					}
					Ret.Add(NewTraceInstance);
				}
			}
			else
			{
				UE_LOG(LogGCS_Collision, Warning, TEXT("Failed to create default trace instance of %s,missing valid trace class."), *Def.TraceTag.ToString())
			}
		}
	}
	return Ret;
}

TArray<UGCS_CollisionTraceInstance*> UGCS_CollisionSystemComponent::GetTraceInstances() const
{
	return TraceInstances;
}

void UGCS_CollisionSystemComponent::GetTraceInstanceByTag(FGameplayTag TraceCollisionToFind, UGCS_CollisionTraceInstance*& FoundTraceCollision)
{
	FoundTraceCollision = nullptr;
	for (int32 i = 0; i < TraceInstances.Num(); i++)
	{
		if (TraceInstances[i])
		{
			if (TraceInstances[i]->TraceGameplayTag == TraceCollisionToFind)
			{
				FoundTraceCollision = TraceInstances[i];
				return;
			}
		}
	}
}

void UGCS_CollisionSystemComponent::GetTraceInstanceByClass(TSubclassOf<class UGCS_CollisionTraceInstance> TraceToFind, UGCS_CollisionTraceInstance*& FoundTrace)
{
	FoundTrace = nullptr;
	if (TraceToFind)
	{
		if (TraceInstances.Num() > 0)
		{
			for (int32 i = 0; i < TraceInstances.Num(); i++)
			{
				if (TraceInstances[i])
				{
					if (TraceInstances[i]->GetClass() == TraceToFind)
					{
						FoundTrace = TraceInstances[i];
						return;
					}
				}
			}
		}
	}
}

void UGCS_CollisionSystemComponent::ToggleTraceInstancesState(FGameplayTagContainer TracesToControl, bool bNewState)
{
	for (UGCS_CollisionTraceInstance* TraceInstance : TraceInstances)
	{
		if (TraceInstance->TraceGameplayTag.IsValid() && TracesToControl.HasTagExact(TraceInstance->TraceGameplayTag))
		{
			TraceInstance->ToggleTraceState(bNewState);
		}
	}
}

void UGCS_CollisionSystemComponent::RemoveTraceFromCreatedTraces(UGCS_CollisionTraceInstance* TraceToRemove)
{
	if (IsValid(TraceToRemove) && TraceInstances.Contains(TraceToRemove))
	{
		TraceInstances.Remove(TraceToRemove);
		FGCS_TraceInstancesCache& Cache = CachedTraceInstances.FindOrAdd(TraceToRemove->GetClass());
		TraceToRemove->OnTraceEndPlay();
		Cache.Instances.Add(TraceToRemove);
		check(Cache.Instances.Contains(TraceToRemove))
	}
}

void UGCS_CollisionSystemComponent::CreateDefaultTraceInstances()
{
	if (UMeshComponent* PrimitiveComp = UGCS_CombatFunctionLibrary::GetMainMeshComponent(GetOwner()))
	{
		CreateTraceInstances(TraceDefinitions, PrimitiveComp);
	}
}

void UGCS_CollisionSystemComponent::ClearTraceInstances()
{
	TraceInstances.Empty();
	for (TTuple<TSubclassOf<UGCS_CollisionTraceInstance>, FGCS_TraceInstancesCache>& Pair : CachedTraceInstances)
	{
		Pair.Value.Instances.Empty();
	}
	CachedTraceInstances.Empty();
}

void UGCS_CollisionSystemComponent::TickActiveTraces(float DeltaTime)
{
	for (int32 i = 0; i < TraceInstances.Num(); i++)
	{
		if (TraceInstances[i]->bTraceActive)
		{
			TraceInstances[i]->OnTraceTick(DeltaTime);
		}
	}
}

void UGCS_CollisionSystemComponent::OnTraceInstanceStateChanged_Implementation(UGCS_CollisionTraceInstance* TraceInstance, bool bNewState)
{
	OnTraceInstanceStateChangedEvent.Broadcast(TraceInstance, bNewState);
}

void UGCS_CollisionSystemComponent::OnTraceInstanceHit_Implementation(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	OnTraceInstanceHitEvent.Broadcast(TraceInstance, HitResult);
}

void UGCS_CollisionSystemComponent::OnBeTraceInstanceHit_Implementation(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	OnBeTraceInstanceHitEvent.Broadcast(TraceInstance, HitResult);
}
