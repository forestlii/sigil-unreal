// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemSlotCollection.h"
#include "GameFramework/Actor.h"
#include "GIS_InventorySystemComponent.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_LogChannels.h"
#include "Misc/DataValidation.h"
#include "UObject/ObjectSaveContext.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemSlotCollection)

TSubclassOf<UGIS_ItemCollection> UGIS_ItemSlotCollectionDefinition::GetCollectionInstanceClass() const
{
	return UGIS_ItemSlotCollection::StaticClass();
}

bool UGIS_ItemSlotCollectionDefinition::IsValidSlotIndex(int32 SlotIndex) const
{
	return SlotDefinitions.IsValidIndex(SlotIndex);
}

int32 UGIS_ItemSlotCollectionDefinition::GetIndexOfSlot(const FGameplayTag& SlotName) const
{
	return TagToIndexMap.Contains(SlotName) ? TagToIndexMap[SlotName] : INDEX_NONE;
}

FGameplayTag UGIS_ItemSlotCollectionDefinition::GetSlotOfIndex(int32 SlotIndex) const
{
	return SlotDefinitions.IsValidIndex(SlotIndex) ? SlotDefinitions[SlotIndex].Tag : FGameplayTag::EmptyTag;
}

const TArray<FGIS_ItemSlotDefinition>& UGIS_ItemSlotCollectionDefinition::GetSlotDefinitions() const
{
	return SlotDefinitions;
}

bool UGIS_ItemSlotCollectionDefinition::GetSlotDefinition(int32 SlotIndex, FGIS_ItemSlotDefinition& OutDefinition) const
{
	if (SlotDefinitions.IsValidIndex(SlotIndex))
	{
		OutDefinition = SlotDefinitions[SlotIndex];
		return true;
	}
	return false;
}

bool UGIS_ItemSlotCollectionDefinition::GetSlotDefinition(const FGameplayTag& SlotName, FGIS_ItemSlotDefinition& OutDefinition) const
{
	if (TagToIndexMap.Contains(SlotName))
	{
		check(SlotDefinitions.IsValidIndex(TagToIndexMap[SlotName]));
		OutDefinition = SlotDefinitions[TagToIndexMap[SlotName]];
		return true;
	}
	return false;
}

int32 UGIS_ItemSlotCollectionDefinition::GetSlotIndexWithinGroup(FGameplayTag GroupTag, FGameplayTag SlotTag) const
{
	if (SlotGroupMap.Contains(GroupTag))
	{
		if (SlotGroupMap[GroupTag].SlotToIndexMap.Contains(SlotTag))
		{
			return SlotGroupMap[GroupTag].SlotToIndexMap[SlotTag];
		}
	}
	return INDEX_NONE;
}

UGIS_ItemSlotCollection::UGIS_ItemSlotCollection(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

void UGIS_ItemSlotCollection::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(ThisClass, ItemBySlots)
}

const UGIS_ItemSlotCollectionDefinition* UGIS_ItemSlotCollection::GetMyDefinition() const
{
	return MyDefinition;
}

FGIS_ItemInfo UGIS_ItemSlotCollection::AddItem(const FGIS_ItemInfo& ItemInfo)
{
	if (!ItemInfo.IsValid())
	{
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}
	return AddItem(ItemInfo, GetTargetSlotIndex(ItemInfo.Item));
}

FGIS_ItemInfo UGIS_ItemSlotCollection::AddItem(const FGIS_ItemInfo& ItemInfo, int32 SlotIndex)
{
	FGIS_ItemInfo ItemInfoAdded = AddItemInternal(ItemInfo, SlotIndex);

	if (ItemInfoAdded.Amount < ItemInfo.Amount)
	{
		HandleItemOverflow(ItemInfo, ItemInfoAdded);
	}
	return ItemInfoAdded;
}

void UGIS_ItemSlotCollection::ServerAddItem_Implementation(const FGIS_ItemInfo& ItemInfo, int32 SlotIndex)
{
	AddItem(ItemInfo, SlotIndex);
}

FGIS_ItemInfo UGIS_ItemSlotCollection::AddItemBySlotName(const FGIS_ItemInfo& ItemInfo, FGameplayTag SlotName)
{
	int32 Index = MyDefinition->GetIndexOfSlot(SlotName);
	if (Index <= INDEX_NONE)
	{
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}

	return AddItem(ItemInfo, Index);
}

void UGIS_ItemSlotCollection::ServerAddItemBySlotName_Implementation(const FGIS_ItemInfo& ItemInfo, FGameplayTag SlotName)
{
	AddItemBySlotName(ItemInfo, SlotName);
}

FGIS_ItemInfo UGIS_ItemSlotCollection::RemoveItem(const FGIS_ItemInfo& ItemInfo)
{
	int32 SlotIndex = GetItemSlotIndex(ItemInfo.Item);
	if (SlotIndex == INDEX_NONE)
	{
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}

	return RemoveItem(SlotIndex, ItemInfo.Amount);
}

FGIS_ItemInfo UGIS_ItemSlotCollection::RemoveItem(int32 SlotIndex, int32 Amount)
{
	if (!SlotToStackMap.Contains(SlotIndex))
	{
		return FGIS_ItemInfo(nullptr, 0, this);
	}

	const FGIS_ItemStack* ItemToRemove = Container.FindById(SlotToStackMap[SlotIndex]);

	if (ItemToRemove == nullptr)
	{
		return FGIS_ItemInfo(nullptr, 0, this);
	}

	int32 AmountToRemove = Amount > 0 ? Amount : ItemToRemove->Amount;
	FGIS_ItemInfo Removed = RemoveInternal(FGIS_ItemInfo(ItemToRemove->Item, AmountToRemove, this, ItemToRemove->Id));

	return Removed;
}

bool UGIS_ItemSlotCollection::IsItemFitWithSlot(const UGIS_ItemInstance* Item, int32 SlotIndex) const
{
	if (!IsValid(Item))
	{
		GIS_CLOG(Error, "invalid item")
		return false;
	}
	const TArray<FGIS_ItemSlotDefinition>& SlotDefinitions = MyDefinition->SlotDefinitions;
	if (SlotDefinitions.IsEmpty() || !SlotDefinitions.IsValidIndex(SlotIndex))
	{
		GIS_CLOG(Error, "has empty slots!")
		return false;
	}

	const FGIS_ItemSlotDefinition& SlotDefinition = SlotDefinitions[SlotIndex];

	// doesn't match definition.
	if (!SlotDefinition.MatchItem(Item))
	{
		return false;
	}

	// slot is empty.
	if (!SlotToStackMap.Contains(SlotIndex))
	{
		return true;
	}

	// slot is not empty, check if it can stack.
	if (!Item->IsUnique())
	{
		if (const FGIS_ItemStack* ExistingStack = Container.FindById(SlotToStackMap[SlotIndex]))
		{
			if (ExistingStack->Item->StackableEquivalentTo(Item))
			{
				return true;
			}
		}
	}

	// can't stack, check new item can replace existing item.
	return MyDefinition->bNewItemPriority;
}

int32 UGIS_ItemSlotCollection::GetTargetSlotIndex(const UGIS_ItemInstance* Item) const
{
	const TArray<FGIS_ItemSlotDefinition>& SlotDefinitions = MyDefinition->SlotDefinitions;
	if (SlotDefinitions.IsEmpty())
	{
		GIS_CLOG(Error, "has empty slots!")
		return INDEX_NONE;
	}

	int32 FoundUsedSlot = INDEX_NONE;
	int32 FoundEmptySlot = INDEX_NONE;

	for (int32 i = 0; i < SlotDefinitions.Num(); i++)
	{
		const FGIS_ItemSlotDefinition& SlotDefinition = SlotDefinitions[i];
		if (!SlotDefinition.MatchItem(Item))
		{
			continue;
		}

		FoundUsedSlot = i;

		// i对应的槽为空
		if (!SlotToStackMap.Contains(i))
		{
			if (FoundEmptySlot != INDEX_NONE)
			{
				continue;
			}
			FoundEmptySlot = i;
			continue;
		}

		//非唯一且可堆叠.
		if (!Item->IsUnique())
		{
			if (const FGIS_ItemStack* ExistingStack = Container.FindById(SlotToStackMap[i]))
			{
				if (ExistingStack->Item->StackableEquivalentTo(Item))
				{
					return i;
				}
			}
		}
	}

	if (FoundEmptySlot != INDEX_NONE)
	{
		return FoundEmptySlot;
	}
	return FoundUsedSlot;
}

bool UGIS_ItemSlotCollection::GetItemInfoAtSlot(FGameplayTag SlotTag, FGIS_ItemInfo& OutItemInfo) const
{
	int32 Index = MyDefinition->GetIndexOfSlot(SlotTag);

	if (Index != INDEX_NONE)
	{
		OutItemInfo = GetItemInfoAtSlot(Index);

		return OutItemInfo.IsValid();
	}
	return false;
}

bool UGIS_ItemSlotCollection::FindItemInfoAtSlot(FGameplayTag SlotTag, FGIS_ItemInfo& OutItemInfo) const
{
	return GetItemInfoAtSlot(SlotTag, OutItemInfo);
}

bool UGIS_ItemSlotCollection::GetItemStackAtSlot(FGameplayTag SlotTag, FGIS_ItemStack& OutItemStack) const
{
	int32 Index = MyDefinition->GetIndexOfSlot(SlotTag);

	if (Index != INDEX_NONE)
	{
		OutItemStack = GetItemStackAtSlot(Index);
		return OutItemStack.IsValidStack();
	}
	return false;
}

bool UGIS_ItemSlotCollection::FindItemStackAtSlot(FGameplayTag SlotTag, FGIS_ItemStack& OutItemStack) const
{
	return GetItemStackAtSlot(SlotTag, OutItemStack);
}

FGIS_ItemInfo UGIS_ItemSlotCollection::GetItemInfoAtSlot(int32 SlotIndex) const
{
	if (SlotIndex <= 0) { return FGIS_ItemInfo::None; }

	FGIS_ItemStack ItemStack = GetItemStackAtSlot(SlotIndex);
	if (ItemStack.IsValidStack())
	{
		return FGIS_ItemInfo(ItemStack.Item, ItemStack.Amount, ItemStack.Collection);
	}
	return FGIS_ItemInfo::None;
}

FGIS_ItemStack UGIS_ItemSlotCollection::GetItemStackAtSlot(int32 SlotIndex) const
{
	if (SlotToStackMap.Contains(SlotIndex))
	{
		if (const FGIS_ItemStack* Stack = Container.FindById(SlotToStackMap[SlotIndex]))
		{
			return *Stack;
		}
	}
	return FGIS_ItemStack();
}

FGameplayTag UGIS_ItemSlotCollection::GetItemSlotName(const UGIS_ItemInstance* Item) const
{
	int32 SlotIndex = GetItemSlotIndex(Item);
	return MyDefinition->GetSlotOfIndex(SlotIndex);
}

int32 UGIS_ItemSlotCollection::GetItemSlotIndex(const UGIS_ItemInstance* Item) const
{
	const TArray<FGIS_ItemSlotDefinition>& SlotDefinitions = MyDefinition->SlotDefinitions;

	int32 StackableEquivalentItemIndex = INDEX_NONE;
	for (int i = 0; i < SlotDefinitions.Num(); i++)
	{
		const FGIS_ItemSlotDefinition& SlotDefinition = SlotDefinitions[i];
		if (!SlotDefinition.MatchItem(Item))
		{
			continue;
		}

		if (!SlotToStackMap.Contains(i) || !SlotToStackMap[i].IsValid())
		{
			continue;
		}

		if (const FGIS_ItemStack* Stack = Container.FindById(SlotToStackMap[i]))
		{
			if (Stack->Item == Item)
			{
				return i;
			}

			if (Stack->Item->StackableEquivalentTo(Item))
			{
				StackableEquivalentItemIndex = i;
			}
		}
	}

	return StackableEquivalentItemIndex;
}

FGIS_ItemInfo UGIS_ItemSlotCollection::AddItemInternal(const FGIS_ItemInfo& ItemInfo, int32 SlotIndex)
{
	FGIS_ItemInfo CanAddItemInfo;
	const bool CanAddResult = CanAddItem(ItemInfo, CanAddItemInfo);
	if (!CanAddResult)
	{
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}

	if (SlotIndex == INDEX_NONE)
	{
		GIS_CLOG(Warning, "invalid valid target slot(%d) to put item:%s", SlotIndex, *ItemInfo.GetDebugString())
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}

	FGIS_ItemSlotDefinition SlotDefinition;
	if (!MyDefinition->GetSlotDefinition(SlotIndex, SlotDefinition))
	{
		GIS_CLOG(Verbose, "No slot definition found for slot index:%d", SlotIndex)
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}

	if (!MyDefinition->bNewItemPriority && CanAddItemInfo.Item->IsUnique() && SlotToStackMap.Contains(SlotIndex) && SlotToStackMap[SlotIndex].IsValid())
	{
		GIS_CLOG(Verbose, "Can't add item info because the target slot:%s was occupied. ItemInfo:%s.", *SlotDefinition.Tag.ToString(), *ItemInfo.GetDebugString())
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}

	// similar item amount for this item.
	int32 CurrentAmount = GetItemAmount(CanAddItemInfo.Item.Get());

	FGIS_ItemInfo SetItemInfo;
	if (SetItemAmount(FGIS_ItemInfo(CanAddItemInfo.Amount + CurrentAmount, CanAddItemInfo), SlotIndex, true, SetItemInfo))
	{
		return SetItemInfo;
	}
	return FGIS_ItemInfo(ItemInfo.Item, 0, this);
}

int32 UGIS_ItemSlotCollection::StackIdToSlotIndex(FGuid InStackId) const
{
	if (!InStackId.IsValid())
	{
		return INDEX_NONE;
	}
	if (StackToIdxMap.Contains(InStackId))
	{
		return StackToIdxMap[InStackId];
	}
	return INDEX_NONE;
}

int32 UGIS_ItemSlotCollection::SlotIndexToStackIndex(int32 InSlotIndex) const
{
	if (SlotToStackMap.Contains(InSlotIndex))
	{
		return Container.IndexOfById(SlotToStackMap[InSlotIndex]);
	}
	return INDEX_NONE;
}

FGuid UGIS_ItemSlotCollection::SlotIndexToStackId(int32 InSlotIndex) const
{
	return SlotToStackMap.Contains(InSlotIndex) && SlotToStackMap[InSlotIndex].IsValid() ? SlotToStackMap[InSlotIndex] : FGuid();
}

void UGIS_ItemSlotCollection::OnRep_ItemsBySlot()
{
}

void UGIS_ItemSlotCollection::SetDefinition(const UGIS_ItemCollectionDefinition* NewDefinition)
{
	Super::SetDefinition(NewDefinition);
	check(OwningInventory != nullptr)
	check(Definition != nullptr)
	if (bInitialized)
	{
		MyDefinition = CastChecked<UGIS_ItemSlotCollectionDefinition>(Definition);
		if (OwningInventory->GetOwnerRole() >= ROLE_Authority)
		{
			if (MyDefinition->SlotDefinitions.Num() == 0)
			{
				GIS_CLOG(Error, "has empty slots")
			}
		}
	}
}

void UGIS_ItemSlotCollection::OnPreItemStackAdded(const FGIS_ItemStack& Stack, int32 Idx)
{
	Super::OnPreItemStackAdded(Stack, Idx);
}

void UGIS_ItemSlotCollection::OnItemStackAdded(const FGIS_ItemStack& Stack)
{
	check(Stack.IsValidStack())
	if (OwningInventory)
	{
		SlotToStackMap.Add(Stack.Index, Stack.Id);
	}
	Super::OnItemStackAdded(Stack);
}

void UGIS_ItemSlotCollection::OnItemStackRemoved(const FGIS_ItemStack& Stack)
{
	if (OwningInventory)
	{
		SlotToStackMap.Remove(Stack.Index);
	}
	Super::OnItemStackRemoved(Stack);
}

bool UGIS_ItemSlotCollection::SetItemAmount(const FGIS_ItemInfo& ItemInfo, int32 SlotIndex, bool RemovePreviousItem, FGIS_ItemInfo& ItemInfoAdded)
{
	if (!OwningInventory->GetOwner()->HasAuthority())
	{
		GIS_CLOG(Warning, "has no authority!");
		return false;
	}

	int32 Amount = ItemInfo.Amount;

	int32 StackIdx = SlotIndexToStackIndex(SlotIndex);

	// Found valid stack
	if (StackIdx != INDEX_NONE)
	{
		const FGIS_ItemStack& CurrentStack = Container.Stacks[StackIdx];

		if (ItemInfo.Item->StackableEquivalentTo(CurrentStack.Item))
		{
			// reduce existing amount to get the amount needed to add.
			Amount -= CurrentStack.Amount;
		}
		else if (RemovePreviousItem)
		{
			FGIS_ItemInfo RemovedItem = RemoveItem(FGIS_ItemInfo(CurrentStack));
			if (RemovedItem.Amount > 0)
			{
				FGIS_ItemInfo CanAddItemInfo;
				if (MyDefinition->bTryGivePrevItemToNewItemCollection && ItemInfo.ItemCollection != nullptr && ItemInfo.ItemCollection->CanAddItem(RemovedItem, CanAddItemInfo))
				{
					GIS_CLOG(Verbose, "An existing item has been replaced by a new one, and the old one has been added to the collection where the new item coming from. prev:%s new:%s",
					         *RemovedItem.GetDebugString(), *ItemInfo.GetDebugString())
					ItemInfo.ItemCollection->AddItem(CanAddItemInfo);
				}
				else
				{
					GIS_CLOG(Verbose, "An item has been replaced by a new one, and the old one has gone forever! prev: %s new:%s",
					         *RemovedItem.GetDebugString(), *ItemInfo.GetDebugString())
				}
			}
		}
		else
		{
			return false;
		}
	}

	// Not found, add new item.
	ItemInfoAdded = AddInternal(FGIS_ItemInfo(Amount, SlotIndex, ItemInfo));

	return true;
}

#if WITH_EDITOR
void UGIS_ItemSlotCollectionDefinition::PreSave(FObjectPreSaveContext SaveContext)
{
	// remove repeated slot by name.
	{
		TArray<FGameplayTag> SlotNames;
		TArray<FGIS_ItemSlotDefinition> Slots;
		for (int32 i = 0; i < SlotDefinitions.Num(); ++i)
		{
			if (!SlotNames.Contains(SlotDefinitions[i].Tag))
			{
				Slots.Add(SlotDefinitions[i]);
				SlotNames.Add(SlotDefinitions[i].Tag);
			}
		}
		SlotDefinitions = Slots;
	}

	IndexToTagMap.Empty();
	TagToIndexMap.Empty();

	for (int32 i = 0; i < SlotDefinitions.Num(); ++i)
	{
		if (!SlotDefinitions[i].Tag.IsValid())
		{
			continue;
		}
		if (!IndexToTagMap.Contains(i))
		{
			IndexToTagMap.Add(i, SlotDefinitions[i].Tag);
		}
		if (!TagToIndexMap.Contains(SlotDefinitions[i].Tag))
		{
			TagToIndexMap.Add(SlotDefinitions[i].Tag, i);
		}
	}

	TArray<FGIS_ItemSlotGroup> Groups;
	SlotGroupMap.Empty();
	for (int32 i = 0; i < SlotGroups.Num(); i++)
	{
		FGIS_ItemSlotGroup Group;
		int32 Idx = 0;
		for (int32 j = 0; j < SlotDefinitions.Num(); j++)
		{
			if (SlotDefinitions[j].Tag.MatchesTag(SlotGroups[i]))
			{
				Group.IndexToSlotMap.Add(Idx, SlotDefinitions[j].Tag);
				Group.SlotToIndexMap.Add(SlotDefinitions[j].Tag, Idx);
				Idx++;
			}
		}
		if (!Group.IndexToSlotMap.IsEmpty())
		{
			SlotGroupMap.Add(SlotGroups[i], Group);
		}
	}

	Super::PreSave(SaveContext);
}
#endif
