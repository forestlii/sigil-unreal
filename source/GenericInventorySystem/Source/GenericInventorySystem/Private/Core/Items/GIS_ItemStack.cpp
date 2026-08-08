// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Items/GIS_ItemStack.h"
#include "Components/ActorComponent.h"
#include "GIS_InventoryMeesages.h"
#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemDefinition.h"
#include "GIS_InventorySystemComponent.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_LogChannels.h"

#include "GIS_ItemSlotCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemStack)

FGuid FGIS_ItemStack::InvalidId = FGuid(0, 0, 0, 0);

FGIS_ItemStack::FGIS_ItemStack()
{
	Item = nullptr;
	Id = InvalidId;
	Amount = 0;
	Collection = nullptr;
	LastObservedAmount = -1;
}

FString FGIS_ItemStack::GetDebugString()
{
	return FString::Format(TEXT("Item({0}),Amount({1}),ID({2})"), {Item && Item->GetDefinition() ? GetNameSafe(Item->GetDefinition()) : TEXT("None"), Amount, Id.ToString()});
}

void FGIS_ItemStack::Initialize(FGuid InStackId, UGIS_ItemInstance* InItem, int32 InAmount, UGIS_ItemCollection* InCollection, int32 InIndex)
{
	Id = InStackId;
	Item = InItem;
	Amount = InAmount;
	Collection = InCollection;
	Index = InIndex;
	LastObservedAmount = InAmount;
}

bool FGIS_ItemStack::IsValidStack() const
{
	return Id.IsValid() && IsValid(Item) && IsValid(Collection) && Amount > 0;
}

void FGIS_ItemStack::Reset()
{
	Id.Invalidate();
	Item = nullptr;
	Amount = 0;
	Collection = nullptr;
	LastObservedAmount = INDEX_NONE;
}

bool FGIS_ItemStack::operator==(const FGIS_ItemStack& Other) const
{
	return Id == Other.Id && Item->GetItemId() == Other.Item->GetItemId();
}

bool FGIS_ItemStack::operator!=(const FGIS_ItemStack& Other) const
{
	return !(operator==(Other));
}

bool FGIS_ItemStack::operator==(const FGuid& OtherId) const
{
	return Id == OtherId;
}

bool FGIS_ItemStack::operator!=(const FGuid& OtherId) const
{
	return Id != OtherId;
}

void FGIS_ItemStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FGIS_ItemStack& Stack = Stacks[Index];

		if (OwningCollection->StackToIdxMap.Contains(Stack.Id))
		{
			OwningCollection->OnItemStackRemoved(Stack);
		}
		else if (OwningCollection->PendingItemStacks.Contains(Stack.Id))
		{
			GIS_OWNED_CLOG(OwningCollection, Warning, "Discard pending item stack(%s).", *OwningCollection->PendingItemStacks[Stack.Id].GetDebugString())
			OwningCollection->PendingItemStacks.Remove(Stack.Id);
		}

		Stack.LastObservedAmount = 0;
	}
}

void FGIS_ItemStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FGIS_ItemStack& Stack = Stacks[Index];

		if (OwningCollection && OwningCollection->IsInitialized() && Stack.IsValidStack())
		{
			OwningCollection->OnItemStackAdded(Stack);
		}
		else if (OwningCollection->PendingItemStacks.Contains(Stack.Id))
		{
			OwningCollection->PendingItemStacks[Stack.Id] = Stack;
		}
		else
		{
			OwningCollection->PendingItemStacks.Add(Stack.Id, Stack);
		}
		Stack.LastObservedAmount = Stack.Amount;
	}
}

void FGIS_ItemStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FGIS_ItemStack& Stack = Stacks[Index];
		check(Stack.LastObservedAmount != INDEX_NONE);

		if (OwningCollection->StackToIdxMap.Contains(Stack.Id)) //Already Added.
		{
			OwningCollection->OnItemStackUpdated(Stack);
		}
		else if (OwningCollection->PendingItemStacks.Contains(Stack.Id)) //In pending list.
		{
			OwningCollection->PendingItemStacks.Emplace(Stack.Id, Stack); // Updated to pending.
		}
		else
		{
			OwningCollection->PendingItemStacks.Add(Stack.Id, Stack); //Add to pending list.
		}
		Stack.LastObservedAmount = Stack.Amount;
	}
}

const FGIS_ItemStack* FGIS_ItemStackContainer::FindById(const FGuid& StackId) const
{
	if (!StackId.IsValid())
	{
		return nullptr;
	}
	return Stacks.FindByPredicate([StackId](const FGIS_ItemStack& Stack)
	{
		return Stack.Id == StackId;
	});
}

const FGIS_ItemStack* FGIS_ItemStackContainer::FindByItemId(const FGuid& ItemId) const
{
	if (!ItemId.IsValid())
	{
		return nullptr;
	}
	return Stacks.FindByPredicate([ItemId](const FGIS_ItemStack& Stack)
	{
		return Stack.Item->GetItemId() == ItemId;
	});
}

int32 FGIS_ItemStackContainer::IndexOfById(const FGuid& StackId) const
{
	if (!StackId.IsValid())
	{
		return INDEX_NONE;
	}
	return Stacks.IndexOfByPredicate([StackId](const FGIS_ItemStack& Stack)
	{
		return Stack.Id == StackId;
	});
}

int32 FGIS_ItemStackContainer::IndexOfByItemId(const FGuid& ItemId) const
{
	return Stacks.IndexOfByPredicate([ItemId](const FGIS_ItemStack& Stack)
	{
		return Stack.Item->GetItemId() == ItemId;
	});
}

int32 FGIS_ItemStackContainer::IndexOfByIds(const FGuid& StackId, const FGuid& ItemId) const
{
	if (!StackId.IsValid() || !ItemId.IsValid())
	{
		return INDEX_NONE;
	}
	return Stacks.IndexOfByPredicate([StackId,ItemId](const FGIS_ItemStack& Stack)
	{
		return Stack.Id == StackId && Stack.Item->GetItemId() == ItemId;
	});
}
