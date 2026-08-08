// Copyright 2025 RedMoonGames All Rights Reserved.


#include "CombatFlow/GCS_AttackResult.h"

#include "GameplayEffect.h"
#include "GCS_LogChannels.h"
#include "GCS_CombatSystemComponent.h"
#include "CombatFlow/GCS_CombatFlow.h"


FGCS_AttackResultContainer::FGCS_AttackResultContainer(): CombatFlow(nullptr),CombatSystemComponent(nullptr), MaxSize(5)
{
}

FGCS_AttackResultContainer::FGCS_AttackResultContainer(UGCS_CombatFlow* InCombatFlow, int32 InMaxSize): CombatFlow(InCombatFlow),CombatSystemComponent(nullptr), MaxSize(InMaxSize)
{
}

FGCS_AttackResultContainer::FGCS_AttackResultContainer(UGCS_CombatSystemComponent* InCombatSystemComponent, int32 InMaxSize):CombatFlow(nullptr),CombatSystemComponent(InCombatSystemComponent),MaxSize(InMaxSize)
{
}

void FGCS_AttackResultContainer::AddEntry(FGCS_AttackResult& NewEntry)
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
		UE_LOG(LogGCS, Error, TEXT("Missing Combat Flow on %s's combat system component."), *CombatSystemComponent->GetOwner()->GetName());
	}

	MarkItemDirty(NewEntry);
}

void FGCS_AttackResultContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
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

void FGCS_AttackResultContainer::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
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
