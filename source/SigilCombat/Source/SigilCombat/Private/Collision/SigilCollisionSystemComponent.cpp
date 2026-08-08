// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Collision/SigilCollisionSystemComponent.h"
#include "SigilCombatLogChannels.h"
#include "Collision/SigilCollisionTraceInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "TargetingSystem/TargetingPreset.h"
#include "Utility/SigilCombatFunctionLibrary.h"

USigilCollisionSystemComponent::USigilCollisionSystemComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

USigilCollisionSystemComponent* USigilCollisionSystemComponent::GetCollisionSystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<USigilCollisionSystemComponent>() : nullptr;
}

bool USigilCollisionSystemComponent::FindCollisionSystemComponent(const AActor* Actor, USigilCollisionSystemComponent*& Component)
{
	Component = GetCollisionSystemComponent(Actor);
	return Component != nullptr;
}

void USigilCollisionSystemComponent::OnInitialize_Implementation()
{
	CreateDefaultTraceInstances();
}

void USigilCollisionSystemComponent::OnDeinitialize_Implementation()
{
	ClearTraceInstances();
}

// Called when the game starts
void USigilCollisionSystemComponent::BeginPlay()
{
	if (bAutoInitialize)
	{
		OnInitialize();
	}
	Super::BeginPlay();
}

void USigilCollisionSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bAutoInitialize)
	{
		OnDeinitialize();
	}
	Super::EndPlay(EndPlayReason);
}


USigilCollisionTraceInstance* USigilCollisionSystemComponent::GetOrCreateTraceInstance(TSubclassOf<USigilCollisionTraceInstance> TraceClass)
{
	USigilCollisionTraceInstance* RetInstance = nullptr;
	if (IsValid(TraceClass))
	{
		FSigilTraceInstancesCache& Cache = CachedTraceInstances.FindOrAdd(TraceClass);
		if (Cache.Instances.IsEmpty())
		{
			RetInstance = NewObject<USigilCollisionTraceInstance>(GetOwner(), TraceClass);
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
void USigilCollisionSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickActiveTraces(DeltaTime);
}

TArray<USigilCollisionTraceInstance*> USigilCollisionSystemComponent::CreateTraceInstances(TArray<FSigilCollisionTraceDefinition> Definitions, UPrimitiveComponent* PrimitiveComponent)
{
	TArray<USigilCollisionTraceInstance*> Ret;
	if (PrimitiveComponent)
	{
		for (const FSigilCollisionTraceDefinition& Def : Definitions)
		{
			if (Def.TraceClass.IsNull() || !Def.TraceTag.IsValid())
				continue;

			TArray<FName> PrimitiveSocketNames = Def.SocketNames.IsEmpty()
				                                     ? USigilCombatFunctionLibrary::GetSocketNamesWithPrefix(PrimitiveComponent, Def.SocketPrefix.ToString(), ESearchCase::IgnoreCase)
				                                     : Def.SocketNames;

			if (TSubclassOf<USigilCollisionTraceInstance> TraceClass = Def.TraceClass.LoadSynchronous())
			{
				if (USigilCollisionTraceInstance* NewTraceInstance = GetOrCreateTraceInstance(TraceClass))
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
				UE_LOG(LogSigilCombat_Collision, Warning, TEXT("Failed to create default trace instance of %s,missing valid trace class."), *Def.TraceTag.ToString())
			}
		}
	}
	return Ret;
}

TArray<USigilCollisionTraceInstance*> USigilCollisionSystemComponent::GetTraceInstances() const
{
	return TraceInstances;
}

void USigilCollisionSystemComponent::GetTraceInstanceByTag(FGameplayTag TraceCollisionToFind, USigilCollisionTraceInstance*& FoundTraceCollision)
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

void USigilCollisionSystemComponent::GetTraceInstanceByClass(TSubclassOf<class USigilCollisionTraceInstance> TraceToFind, USigilCollisionTraceInstance*& FoundTrace)
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

void USigilCollisionSystemComponent::ToggleTraceInstancesState(FGameplayTagContainer TracesToControl, bool bNewState)
{
	for (USigilCollisionTraceInstance* TraceInstance : TraceInstances)
	{
		if (TraceInstance->TraceGameplayTag.IsValid() && TracesToControl.HasTagExact(TraceInstance->TraceGameplayTag))
		{
			TraceInstance->ToggleTraceState(bNewState);
		}
	}
}

void USigilCollisionSystemComponent::RemoveTraceFromCreatedTraces(USigilCollisionTraceInstance* TraceToRemove)
{
	if (IsValid(TraceToRemove) && TraceInstances.Contains(TraceToRemove))
	{
		TraceInstances.Remove(TraceToRemove);
		FSigilTraceInstancesCache& Cache = CachedTraceInstances.FindOrAdd(TraceToRemove->GetClass());
		TraceToRemove->OnTraceEndPlay();
		Cache.Instances.Add(TraceToRemove);
		check(Cache.Instances.Contains(TraceToRemove))
	}
}

void USigilCollisionSystemComponent::CreateDefaultTraceInstances()
{
	if (UMeshComponent* PrimitiveComp = USigilCombatFunctionLibrary::GetMainMeshComponent(GetOwner()))
	{
		CreateTraceInstances(TraceDefinitions, PrimitiveComp);
	}
}

void USigilCollisionSystemComponent::ClearTraceInstances()
{
	TraceInstances.Empty();
	for (TTuple<TSubclassOf<USigilCollisionTraceInstance>, FSigilTraceInstancesCache>& Pair : CachedTraceInstances)
	{
		Pair.Value.Instances.Empty();
	}
	CachedTraceInstances.Empty();
}

void USigilCollisionSystemComponent::TickActiveTraces(float DeltaTime)
{
	for (int32 i = 0; i < TraceInstances.Num(); i++)
	{
		if (TraceInstances[i]->bTraceActive)
		{
			TraceInstances[i]->OnTraceTick(DeltaTime);
		}
	}
}

void USigilCollisionSystemComponent::OnTraceInstanceStateChanged_Implementation(USigilCollisionTraceInstance* TraceInstance, bool bNewState)
{
	OnTraceInstanceStateChangedEvent.Broadcast(TraceInstance, bNewState);
}

void USigilCollisionSystemComponent::OnTraceInstanceHit_Implementation(USigilCollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	OnTraceInstanceHitEvent.Broadcast(TraceInstance, HitResult);
}

void USigilCollisionSystemComponent::OnBeTraceInstanceHit_Implementation(USigilCollisionTraceInstance* TraceInstance, const FHitResult& HitResult)
{
	OnBeTraceInstanceHitEvent.Broadcast(TraceInstance, HitResult);
}
