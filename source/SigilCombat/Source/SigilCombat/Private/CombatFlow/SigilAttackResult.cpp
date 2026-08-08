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
	if (Results.Num() >= 5)
	{
		Results.RemoveAtSwap(0);
		MarkArrayDirty();
	}

	Results.Add(NewEntry);
	check(CombatSystemComponent != nullptr && CombatSystemComponent->GetOwner());

	if (CombatSystemComponent->GetCombatFlow())
	{
		CombatSystemComponent->GetCombatFlow()->HandleAttackResult(NewEntry);
		NewEntry.bConsumed = true;
	}
	else
	{
		UE_LOG(LogSigilCombat, Error, TEXT("Missing Combat Flow on %s's combat system component."), *CombatSystemComponent->GetOwner()->GetName());
	}

	MarkItemDirty(NewEntry);
}

void FSigilAttackResultContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		if (!Results[Index].bConsumed)
		{
			CombatSystemComponent->GetCombatFlow()->HandleAttackResult(Results[Index]);
			Results[Index].bConsumed = true;
		}
	}
}

void FSigilAttackResultContainer::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		if (!Results[Index].bConsumed)
		{
			CombatSystemComponent->GetCombatFlow()->HandleAttackResult(Results[Index]);
			Results[Index].bConsumed = true;
		}
	}
}
