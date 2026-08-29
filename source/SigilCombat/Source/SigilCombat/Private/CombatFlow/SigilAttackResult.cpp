// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "CombatFlow/SigilAttackResult.h"

#include "GameplayEffect.h"
#include "SigilCombatLogChannels.h"
#include "SigilCombatSystemComponent.h"
#include "CombatFlow/SigilCombatFlow.h"


FSigilAttackResultContainer::FSigilAttackResultContainer(): CombatFlow(nullptr),CombatSystemComponent(nullptr), MaxSize(5)
{
}

FSigilAttackResultContainer::FSigilAttackResultContainer(USigilCombatFlow* InCombatFlow, int32 InMaxSize): CombatFlow(InCombatFlow),CombatSystemComponent(nullptr), MaxSize(InMaxSize)
{
}

FSigilAttackResultContainer::FSigilAttackResultContainer(USigilCombatSystemComponent* InCombatSystemComponent, int32 InMaxSize):CombatFlow(nullptr),CombatSystemComponent(InCombatSystemComponent),MaxSize(InMaxSize)
{
}

void FSigilAttackResultContainer::AddEntry(FSigilAttackResult& NewEntry)
{
	check(CombatSystemComponent != nullptr && CombatSystemComponent->GetOwner());

	// Trim oldest entries first, preserving order (Results is a chronological ring).
	const int32 EffectiveMaxSize = FMath::Max(MaxSize, 1);
	while (Results.Num() >= EffectiveMaxSize)
	{
		Results.RemoveAt(0, EAllowShrinking::No);
		MarkArrayDirty();
	}

	// Only the element that lives inside the array may be marked dirty; the caller's payload must
	// never carry FastArray replication metadata (it may be reused for the next attack).
	FSigilAttackResult& AddedEntry = Results.Add_GetRef(NewEntry);
	AddedEntry.ReplicationID = INDEX_NONE;
	AddedEntry.ReplicationKey = INDEX_NONE;

	USigilCombatFlow* Flow = CombatSystemComponent->IsCombatFlowReady() ? CombatSystemComponent->GetCombatFlow() : nullptr;

	// Finish every write to the array element *before* handing control to the (Blueprint-overridable) flow:
	// a processor may call RegisterAttackResult again, which can reallocate or trim Results and invalidate AddedEntry.
	AddedEntry.bConsumed = (Flow != nullptr);
	MarkItemDirty(AddedEntry);
	FSigilAttackResult DispatchCopy = AddedEntry;
	DispatchCopy.bConsumed = false;

	if (Flow)
	{
		Flow->HandleAttackResult(DispatchCopy);
	}
	else
	{
		UE_LOG(LogSigilCombat, Error, TEXT("Combat Flow on %s's combat system component is missing or not initialized; attack result kept pending."), *CombatSystemComponent->GetOwner()->GetName());
	}
}

void FSigilAttackResultContainer::ConsumeEntry(int32 Index)
{
	if (!Results.IsValidIndex(Index) || Results[Index].bConsumed)
	{
		return;
	}
	if (!IsValid(CombatSystemComponent) || !CombatSystemComponent->IsCombatFlowReady())
	{
		// CombatFlow has not replicated / initialized yet (its RepNotify runs after FastArray callbacks in the same bunch);
		// the entry stays unconsumed and is picked up by ConsumePendingEntries() from OnRep_CombatFlow.
		return;
	}
	USigilCombatFlow* Flow = CombatSystemComponent->GetCombatFlow();

	// Mark first, dispatch a copy: the flow may mutate this container re-entrantly.
	Results[Index].bConsumed = true;
	FSigilAttackResult DispatchCopy = Results[Index];
	DispatchCopy.bConsumed = false;
	Flow->HandleAttackResult(DispatchCopy);
}

void FSigilAttackResultContainer::ConsumePendingEntries()
{
	for (int32 Index = 0; Index < Results.Num(); ++Index)
	{
		ConsumeEntry(Index);
	}
}

void FSigilAttackResultContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		ConsumeEntry(Index);
	}
}

void FSigilAttackResultContainer::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		ConsumeEntry(Index);
	}
}
