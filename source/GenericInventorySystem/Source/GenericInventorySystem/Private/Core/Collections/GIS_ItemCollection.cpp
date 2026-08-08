// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemCollection.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_InventoryMeesages.h"
#include "GIS_InventorySubsystem.h"
#include "GIS_InventoryTags.h"
#include "Items/GIS_ItemInstance.h"
#include "Items/GIS_ItemDefinition.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GIS_ItemRestriction.h"
#include "GIS_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemCollection)

bool UGIS_ItemCollectionDefinition::IsSupportedForNetworking() const
{
	return true;
}

TSubclassOf<UGIS_ItemCollection> UGIS_ItemCollectionDefinition::GetCollectionInstanceClass() const
{
	return UGIS_ItemCollection::StaticClass();
}

UGIS_ItemCollection::GIS_CollectionNotifyLocker::GIS_CollectionNotifyLocker(UGIS_ItemCollection& InItemCollection): ItemCollection(InItemCollection)
{
	ItemCollection.NotifyLocker++;
}

UGIS_ItemCollection::GIS_CollectionNotifyLocker::~GIS_CollectionNotifyLocker()
{
	ItemCollection.NotifyLocker--;
}

UGIS_ItemCollection::UGIS_ItemCollection(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), Container(this)
{
}

void UGIS_ItemCollection::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Container);

	//fix: https://forums.unrealengine.com/t/subobject-replication-for-blueprint-child-class/106205/4
	UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>(this->GetClass());
	if (bpClass != nullptr)
	{
		bpClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
}

bool UGIS_ItemCollection::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* Owner = CastChecked<AActor>(GetOuter());

	bool bProcessed = false;

	FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
	if (Context != nullptr)
	{
		for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
		{
			if (Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateFunction(Owner, Function))
			{
				Driver.NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
				bProcessed = true;
			}
		}
	}

	return bProcessed;
}

int32 UGIS_ItemCollection::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		// This handles absorbing authority/cosmetic
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}
	check(GetOuter() != nullptr);
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UGIS_ItemCollection::IsInitialized() const
{
	return bInitialized;
}

FString UGIS_ItemCollection::GetCollectionName() const
{
	if (Definition)
	{
		if (Definition->CollectionTag.IsValid())
		{
			TArray<FName> TagNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(Definition->CollectionTag, TagNames);
			if (!TagNames.IsEmpty())
			{
				return FString::Format(TEXT("{0} Collection"), {TagNames.Last().ToString()});
			}
		}
		return FString::Format(TEXT("{0}"), {GetNameSafe(this)});
	}
	return TEXT("Invalid Collection!!!");
}

FString UGIS_ItemCollection::GetDebugString() const
{
	return FString::Format(TEXT("{0}({1})"), {GetCollectionName(), GetNameSafe(OwningInventory->GetOwner())});
}

bool UGIS_ItemCollection::HasItem(const UGIS_ItemInstance* Item, int32 Amount, bool SimilarItem) const
{
	if (Item == nullptr) { return false; }

	return GetItemAmount(Item, SimilarItem) >= Amount;
}

FGIS_ItemInfo UGIS_ItemCollection::AddItem(const FGIS_ItemInfo& ItemInfo)
{
	//The actually added item info; 实际添加的道具信息
	FGIS_ItemInfo ItemInfoAdded = FGIS_ItemInfo(ItemInfo.Item, 0, this);

	FGIS_ItemInfo CanAddItemInfo;
	if (CanAddItem(ItemInfo, CanAddItemInfo))
	{
		//非唯一道具或数量为一，直接加.
		if (!CanAddItemInfo.Item->IsUnique() || CanAddItemInfo.Amount <= 1)
		{
			//要添加的信息
			FGIS_ItemInfo ItemInfoToAdd(CanAddItemInfo.Item, CanAddItemInfo.Amount, ItemInfo);
			ItemInfoAdded = AddInternal(ItemInfoToAdd);
		}
		else //unique item can stack.
		{
			//先加1个
			FGIS_ItemInfo OriginalResult = AddItem(FGIS_ItemInfo(CanAddItemInfo.Item, 1, ItemInfo));
			// 遍历加入数量为1的道具, 每个都有不同的GUID
			for (int32 i = 1; i < CanAddItemInfo.Amount; i++)
			{
				UGIS_ItemInstance* DuplicatedItem = UGIS_InventorySubsystem::Get(GetWorld())->DuplicateItem(OwningInventory->GetOwner(), CanAddItemInfo.Item);
				check(DuplicatedItem && DuplicatedItem->IsItemValid())
				AddItem(FGIS_ItemInfo(DuplicatedItem, 1, ItemInfo));
			}
			ItemInfoAdded = FGIS_ItemInfo(CanAddItemInfo.Amount, OriginalResult);
		}
	}

	// overflow
	if (ItemInfoAdded.Amount < ItemInfo.Amount)
	{
		HandleItemOverflow(ItemInfo, ItemInfoAdded);
	}

	return ItemInfoAdded;
}

int32 UGIS_ItemCollection::AddItems(const TArray<FGIS_ItemInfo>& ItemInfos)
{
	int32 TotalAdded = 0;
	for (int32 i = 0; i < ItemInfos.Num(); i++)
	{
		TotalAdded += AddItem(ItemInfos[i]).Amount;
	}
	return TotalAdded;
}

FGIS_ItemInfo UGIS_ItemCollection::AddItem(UGIS_ItemInstance* Item, int32 Amount)
{
	return AddItem(FGIS_ItemInfo(Item, Amount));
}

bool UGIS_ItemCollection::CanAddItem(const FGIS_ItemInfo& Input, FGIS_ItemInfo& Output)
{
	if (!bInitialized || Input.Item == nullptr || !Input.Item->IsItemValid() || Input.Amount < 1)
	{
		return false;
	}

	FGIS_ItemInfo ModifiedInput = Input;

	/*
	 * Documentation: For none-unique item(means stackable), if you want to add 2 apples with id x, there are following case:
	 * 1.There are already 10 apples with id x in your inventory,this will turn your "add 2 apples of id x" request into "add 2 apples of id y", resulting 12 apples with id y in your inventory.
	 * 2.There's no any apples in your inventory, resulting in 2 apples with id x in your inventory.
	 * 文档: 道具不唯一. 举例: 你要加2个苹果(ID是X)到库存，且苹果定义上它不是唯一的（即可叠加）。
	 * 如果1：库存里如果已经有10个苹果(ID是Y)。就把 “你要加2个ID为X的苹果” 变成“你要加2个ID为Y的苹果”.
	 * 如果2：库存里如果没有任何苹果。那么就不变，其结果就是往库存里加2个ID是X的苹果.
	 */
	if (!ModifiedInput.Item->IsUnique())
	{
		FGIS_ItemInfo SimilarItemInfo;
		if (GetItemInfo(ModifiedInput.Item, SimilarItemInfo))
		{
			//修改要加入的道具信息为相似道具、但保留Amount.
			ModifiedInput = FGIS_ItemInfo(Input.Amount, SimilarItemInfo);
		}
	}

	if (ModifiedInput.Item->GetOwningCollection() != nullptr && ModifiedInput.Item->GetOwningCollection() != this)
	{
		UGIS_ItemInstance* DuplicatedItem = UGIS_InventorySubsystem::Get(GetWorld())->DuplicateItem(OwningInventory->GetOwner(), ModifiedInput.Item);
		check(DuplicatedItem && DuplicatedItem->IsItemValid())
		GIS_LOG(Verbose, "The Item(%s) was duplicated, and the duplicated one was added because the item is already part of another collection on actor %s.", *Input.GetDebugString(),
		        *GetNameSafe(Input.Item->GetOuter()))
		ModifiedInput = FGIS_ItemInfo(DuplicatedItem, Input.Amount, Input);
	}

	if (ModifiedInput.Item->GetOuter() != nullptr && ModifiedInput.Item->GetOuter() != OwningInventory->GetOwner())
	{
		UGIS_ItemInstance* DuplicatedItem = UGIS_InventorySubsystem::Get(GetWorld())->DuplicateItem(OwningInventory->GetOwner(), ModifiedInput.Item, false);
		check(DuplicatedItem && DuplicatedItem->IsItemValid())
		GIS_LOG(Verbose, "The Item(%s) was duplicated, and the duplicated one was added because the item was created by another actor %s.", *Input.GetDebugString(),
		        *GetNameSafe(Input.Item->GetOuter()))
		ModifiedInput = FGIS_ItemInfo(DuplicatedItem, Input.Amount, Input);
	}

	for (int32 i = 0; i < Definition->Restrictions.Num(); i++)
	{
		auto Restriction = Definition->Restrictions[i];
		if (Restriction && !Restriction->CanAddItem(ModifiedInput, this))
		{
			GIS_CLOG(Warning, "can't add item(%s) due to restriction:%s", *Input.GetDebugString(), *Restriction->GetClass()->GetName())
			return false;
		}
	}

	Output = ModifiedInput;
	return true;
}

bool UGIS_ItemCollection::CanItemStack(const FGIS_ItemInfo& ItemInfo, const FGIS_ItemStack& ItemStack) const
{
	return !ItemInfo.Item->IsUnique() && ItemStack.Item->StackableEquivalentTo(ItemInfo.Item);
}

bool UGIS_ItemCollection::RemoveItemCondition(const FGIS_ItemInfo& ItemInfo, FGIS_ItemInfo& OutItemInfo)
{
	if (!ItemInfo.Item || !ItemInfo.Item->IsItemValid() || ItemInfo.Amount < 1)
	{
		return false;
	}
	OutItemInfo = ItemInfo;

	if (!ItemInfo.Item->IsUnique())
	{
		FGIS_ItemInfo SimilarItemInfo;
		if (GetItemInfo(ItemInfo.Item, SimilarItemInfo))
		{
			OutItemInfo = FGIS_ItemInfo(SimilarItemInfo.Item, ItemInfo.Amount);
		}
	}

	// make sure is this collection.
	if (OutItemInfo.ItemCollection != this)
	{
		OutItemInfo.ItemCollection = this;
	}

	for (int32 i = 0; i < Definition->Restrictions.Num(); i++)
	{
		// Check against src item info in restriction.
		if (!Definition->Restrictions[i]->CanRemoveItem(OutItemInfo))
		{
			return false;
		}
	}

	return true;
}

FGIS_ItemInfo UGIS_ItemCollection::RemoveItem(const FGIS_ItemInfo& ItemInfo)
{
	FGIS_ItemInfo ItemInfoToRemove;
	if (!RemoveItemCondition(ItemInfo, ItemInfoToRemove))
	{
		return FGIS_ItemInfo(ItemInfo.Item, 0, this);
	}
	return RemoveInternal(ItemInfoToRemove);
}

void UGIS_ItemCollection::RemoveAll()
{
	for (int32 i = Container.Stacks.Num() - 1; i >= 0; i--)
	{
		FGIS_ItemStack& ItemStack = Container.Stacks[i];
		RemoveItem(FGIS_ItemInfo(ItemStack.Item, ItemStack.Amount));
	}
}

bool UGIS_ItemCollection::GetItemInfoByStackId(FGuid InStackId, FGIS_ItemInfo& OutItemInfo) const
{
	if (const FGIS_ItemStack* Stack = Container.Stacks.FindByKey(InStackId))
	{
		OutItemInfo = *Stack;
		return true;
	}
	return false;
}

bool UGIS_ItemCollection::FindItemInfoByStackId(FGuid InStackId, FGIS_ItemInfo& OutItemInfo) const
{
	return GetItemInfoByStackId(InStackId, OutItemInfo);
}

bool UGIS_ItemCollection::GetItemInfo(const UGIS_ItemInstance* Item, FGIS_ItemInfo& OutItemInfo) const
{
	if (Item == nullptr)
	{
		return false;
	}

	bool bFoundSimilar = false;
	FGIS_ItemInfo SimilarItemInfo;
	for (int32 i = 0; i < Container.Stacks.Num(); i++)
	{
		const FGIS_ItemStack& ItemStack = Container.Stacks[i];
		//Is not unique but has same definition.
		if (!Item->IsUnique() && ItemStack.Item->GetDefinition() == Item->GetDefinition())
		{
			SimilarItemInfo = FGIS_ItemInfo(ItemStack.Item, ItemStack.Amount, ItemStack.Collection);
			bFoundSimilar = true;
		}

		if (ItemStack.Item->GetItemId() != Item->GetItemId())
		{
			continue;
		}

		//Found unique item with same id.
		OutItemInfo = FGIS_ItemInfo(ItemStack.Item, ItemStack.Amount, ItemStack.Collection);
		return true;
	}

	if (bFoundSimilar)
	{
		OutItemInfo = SimilarItemInfo;
	}
	return bFoundSimilar;
}

bool UGIS_ItemCollection::GetItemInfoByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition>& ItemDefinition, FGIS_ItemInfo& OutItemInfo)
{
	if (ItemDefinition.IsNull())
	{
		return false;
	}

	UGIS_ItemDefinition* LoadedItemDefinition = ItemDefinition.LoadSynchronous();

	if (LoadedItemDefinition == nullptr)
	{
		return false;
	}

	bool bFoundSimilar = false;
	FGIS_ItemInfo SimilarItemInfo;
	for (int32 i = 0; i < Container.Stacks.Num(); i++)
	{
		FGIS_ItemStack& ItemStack = Container.Stacks[i];
		if (ItemStack.Item->GetDefinition() == LoadedItemDefinition)
		{
			SimilarItemInfo = FGIS_ItemInfo(ItemStack.Item, ItemStack.Amount, ItemStack.Collection);
			bFoundSimilar = true;
		}
	}

	if (bFoundSimilar && SimilarItemInfo.IsValid())
	{
		OutItemInfo = SimilarItemInfo;
		return true;
	}
	return false;
}

bool UGIS_ItemCollection::GetItemInfosByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition>& ItemDefinition, TArray<FGIS_ItemInfo>& OutItemInfos)
{
	if (ItemDefinition.IsNull())
	{
		return false;
	}

	UGIS_ItemDefinition* LoadedItemDefinition = ItemDefinition.LoadSynchronous();

	if (LoadedItemDefinition == nullptr)
	{
		return false;
	}

	for (int32 i = 0; i < Container.Stacks.Num(); i++)
	{
		FGIS_ItemStack& ItemStack = Container.Stacks[i];
		if (ItemStack.Item->GetDefinition() == LoadedItemDefinition)
		{
			FGIS_ItemInfo SimilarItemInfo = FGIS_ItemInfo(ItemStack.Item, ItemStack.Amount, ItemStack.Collection);
			OutItemInfos.Add(SimilarItemInfo);
		}
	}
	return OutItemInfos.Num() > 0;
}

int32 UGIS_ItemCollection::GetItemAmount(const UGIS_ItemInstance* Item, bool SimilarItem) const
{
	if (Item == nullptr) { return 0; }

	int32 Count = 0;

	for (int i = 0; i < Container.Stacks.Num(); i++)
	{
		if (Container.Stacks[i].Item && Container.Stacks[i].Item->SimilarTo(Item))
		{
			Count += Container.Stacks[i].Amount;
		}
	}

	return Count;
}

int32 UGIS_ItemCollection::GetItemAmount(TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, bool CountStacks) const
{
	if (ItemDefinition.IsNull())
	{
		return 0;
	}
	UGIS_ItemDefinition* LoadedDefinition = ItemDefinition.LoadSynchronous();
	if (LoadedDefinition == nullptr)
	{
		return 0;
	}
	int32 Count = 0;
	for (int32 i = 0; i < Container.Stacks.Num(); i++)
	{
		if (Container.Stacks[i].Item == nullptr)
		{
			continue;
		}
		if (LoadedDefinition != Container.Stacks[i].Item->GetDefinition())
		{
			continue;
		}
		Count += CountStacks ? 1 : Container.Stacks[i].Amount;
	}

	return Count;
}

TArray<FGIS_ItemInfo> UGIS_ItemCollection::GetAllItemInfos() const
{
	TArray<FGIS_ItemInfo> Infos;
	for (int32 i = 0; i < Container.Stacks.Num(); i++)
	{
		const FGIS_ItemStack& ItemStack = Container.Stacks[i];
		Infos.Add(FGIS_ItemInfo(ItemStack));
	}
	return Infos;
}

TArray<UGIS_ItemInstance*> UGIS_ItemCollection::GetAllItems() const
{
	TArray<UGIS_ItemInstance*> Rets;
	for (int32 i = 0; i < Container.Stacks.Num(); i++)
	{
		if (Container.Stacks[i].IsValidStack())
		{
			Rets.AddUnique(Container.Stacks[i].Item);
		}
	}
	return Rets;
}

const TArray<FGIS_ItemStack>& UGIS_ItemCollection::GetAllItemStacks() const
{
	return Container.Stacks;
}

int32 UGIS_ItemCollection::GetItemStacksNum() const
{
	return Container.Stacks.Num();
}

void UGIS_ItemCollection::AddItemStack(const FGIS_ItemStack& Stack)
{
	check(Stack.IsValidStack())
	int32 Idx = Container.Stacks.AddDefaulted();
	Container.Stacks[Idx] = Stack;
	FGIS_ItemStack& AddedStack = Container.Stacks[Idx];

	OnPreItemStackAdded(Stack, Idx);
	OnItemStackAdded(AddedStack);

	Container.MarkItemDirty(AddedStack);
}

void UGIS_ItemCollection::RemoveItemStackAtIndex(int32 Idx, bool bRemoveFromCollection)
{
	check(Container.Stacks.IsValidIndex(Idx))
	const FGIS_ItemStack RemovedStack = Container.Stacks[Idx];
	OnItemStackRemoved(RemovedStack);
	Container.Stacks.RemoveAt(Idx);
	if (bRemoveFromCollection)
	{
		RemovedStack.Item->UnassignCollection(this);
		if (OwningInventory->IsReplicatedSubObjectRegistered(RemovedStack.Item))
		{
			OwningInventory->RemoveReplicatedSubObject(RemovedStack.Item);
		}
	}
	Container.MarkArrayDirty();
}

void UGIS_ItemCollection::UpdateItemStackAmountAtIndex(int32 Idx, int32 NewAmount)
{
	check(Container.Stacks.IsValidIndex(Idx))
	FGIS_ItemStack& StackToUpdate = Container.Stacks[Idx];
	StackToUpdate.Amount = NewAmount;
	OnItemStackUpdated(StackToUpdate);
	Container.MarkItemDirty(StackToUpdate);
}

void UGIS_ItemCollection::OnPreItemStackAdded(const FGIS_ItemStack& Stack, int32 Idx)
{
	if (IsValid(Stack.Item))
	{
		if (Stack.Item->GetOwningCollection() != this)
		{
			Stack.Item->AssignCollection(this);
		}

		if (!OwningInventory->IsReplicatedSubObjectRegistered(Stack.Item))
		{
			OwningInventory->AddReplicatedSubObject(Stack.Item);
		}
	}
}

void UGIS_ItemCollection::OnItemStackAdded(const FGIS_ItemStack& Stack)
{
	check(Stack.IsValidStack())
	if (OwningInventory)
	{
		StackToIdxMap.Add(Stack.Id, Stack.Index);
		FGIS_InventoryStackUpdateMessage Message;
		Message.Inventory = OwningInventory;
		Message.ChangeType = EGIS_ItemStackChangeType::WasAdded;
		Message.CollectionId = CollectionId;
		Message.Instance = Stack.Item;
		Message.StackId = Stack.Id;
		Message.NewCount = Stack.Amount;
		Message.Delta = Stack.Amount;
		OwningInventory->OnInventoryStackUpdate.Broadcast(Message);

		GIS_CLOG(Verbose, "added item stack:item(%s),amount(%d)", *Stack.Item->GetDefinition()->GetName(), Stack.Amount)
	}
}

void UGIS_ItemCollection::OnItemStackRemoved(const FGIS_ItemStack& Stack)
{
	if (OwningInventory)
	{
		StackToIdxMap.Remove(Stack.Id);
		FGIS_InventoryStackUpdateMessage Message;
		Message.Inventory = OwningInventory;
		Message.ChangeType = EGIS_ItemStackChangeType::WasRemoved;
		Message.CollectionId = CollectionId;
		Message.Instance = Stack.Item;
		Message.StackId = Stack.Id;
		Message.NewCount = 0;
		Message.Delta = -Stack.LastObservedAmount;
		OwningInventory->OnInventoryStackUpdate.Broadcast(Message);

		GIS_CLOG(Verbose, "removed item stack:item(%s),amount(%d)", *Stack.Item->GetDefinition()->GetName(), Stack.Amount)
	}
}

void UGIS_ItemCollection::OnItemStackUpdated(const FGIS_ItemStack& Stack)
{
	if (OwningInventory)
	{
		if (StackToIdxMap.Contains(Stack.Id))
		{
			StackToIdxMap[Stack.Id] = Stack.Index;
		}
		FGIS_InventoryStackUpdateMessage Message;
		Message.Inventory = OwningInventory;
		Message.ChangeType = EGIS_ItemStackChangeType::Changed;
		Message.CollectionId = CollectionId;
		Message.Instance = Stack.Item;
		Message.StackId = Stack.Id;
		Message.NewCount = Stack.Amount;
		Message.Delta = Stack.Amount - Stack.LastObservedAmount;
		OwningInventory->OnInventoryStackUpdate.Broadcast(Message);
		GIS_CLOG(Verbose, "updated item stack:item(%s),amount(%d)", *Stack.Item->GetDefinition()->GetName(), Stack.Amount)
	}
}

void UGIS_ItemCollection::ProcessPendingItemStacks()
{
	if (bInitialized)
	{
		TArray<FGuid> Added;
		for (const TPair<FGuid, FGIS_ItemStack>& Pending : PendingItemStacks)
		{
			if (Pending.Value.IsValidStack())
			{
				Added.AddUnique(Pending.Key);
			}
		}
		for (int32 i = 0; i < Added.Num(); i++)
		{
			FGuid AddedStackId = Added[i];
			const FGIS_ItemStack& AddedStack = PendingItemStacks[AddedStackId];
			OnItemStackAdded(AddedStack);
			GIS_CLOG(Verbose, "added item stack:item(%s),amount(%d) from pending list.", *AddedStack.Item->GetDefinition()->GetName(), AddedStack.Amount)
			PendingItemStacks.Remove(AddedStackId);
		}
	}
}

void UGIS_ItemCollection::SetDefinition(const UGIS_ItemCollectionDefinition* NewDefinition)
{
	check(OwningInventory != nullptr);
	check(NewDefinition != nullptr);
	if (!bInitialized)
	{
		Definition = NewDefinition;
		CollectionTag = Definition->CollectionTag;
		bInitialized = true;
	}
}

void UGIS_ItemCollection::SetCollectionTag(FGameplayTag NewTag)
{
	CollectionTag = NewTag;
}

void UGIS_ItemCollection::SetCollectionId(FGuid NewId)
{
	if (!CollectionId.IsValid())
	{
		CollectionId = NewId;
	}
}

FGIS_ItemInfo UGIS_ItemCollection::AddInternal(const FGIS_ItemInfo& ItemInfo)
{
	bool bFound = false;
	FGIS_ItemStack AddedItemStack;

	if (!ItemInfo.Item->IsUnique())
	{
		// First, trying to add to existing stack.
		int32 TargetStackIdx = Container.IndexOfById(ItemInfo.StackId);
		if (TargetStackIdx != INDEX_NONE)
		{
			check(Container.Stacks.IsValidIndex(TargetStackIdx))
			const FGIS_ItemStack& ExistingStack = Container.Stacks[TargetStackIdx];
			if (ExistingStack.Collection == this && ExistingStack.Item == ItemInfo.Item)
			{
				UpdateItemStackAmountAtIndex(TargetStackIdx, ItemInfo.Amount + ExistingStack.Amount);
				AddedItemStack = ExistingStack;
				bFound = true;
			}
		}

		if (!bFound)
		{
			for (int32 i = 0; i < Container.Stacks.Num(); i++)
			{
				if (CanItemStack(ItemInfo, Container.Stacks[i]) == false)
				{
					continue;
				}

				UpdateItemStackAmountAtIndex(i, ItemInfo.Amount + Container.Stacks[i].Amount);
				AddedItemStack = Container.Stacks[i];
				bFound = true;
				break;
			}
		}
	}

	//新增.
	if (!bFound)
	{
		AddedItemStack.Initialize(FGuid::NewGuid(), ItemInfo.Item, ItemInfo.Amount, this, ItemInfo.Index);
		AddItemStack(AddedItemStack);
	}
	FGIS_ItemInfo AddedItemInfo = FGIS_ItemInfo(ItemInfo.Item, ItemInfo.Amount, this, AddedItemStack.Id);
	return AddedItemInfo;
}

void UGIS_ItemCollection::HandleItemOverflow(const FGIS_ItemInfo& OriginalItemInfo, const FGIS_ItemInfo& ItemInfoAdded)
{
	FGIS_ItemInfo RejectedItemInfo = FGIS_ItemInfo(OriginalItemInfo.Amount - ItemInfoAdded.Amount, OriginalItemInfo);

	FGIS_ItemInfo ReturnedItemInfo;
	if (Definition->OverflowOptions.bReturnOverflow)
	{
		if (OriginalItemInfo.ItemCollection != nullptr && OriginalItemInfo.ItemCollection != this)
		{
			ReturnedItemInfo = OriginalItemInfo.ItemCollection->AddItem(RejectedItemInfo);
		}
	}

	if (Definition->OverflowOptions.bSendRejectedMessage)
	{
		FGIS_InventoryAddItemInfoRejectedMessage Message;
		Message.Inventory = OwningInventory;
		Message.Collection = this;
		Message.OriginalItemInfo = OriginalItemInfo;
		Message.ItemInfoAdded = ItemInfoAdded;
		Message.RejectedItemInfo = RejectedItemInfo;
		Message.ReturnedItemInfo = ReturnedItemInfo;
		OwningInventory->OnInventoryAddItemInfo_Rejected.Broadcast(Message);
	}
	GIS_CLOG(Warning, "try add %s, added %d, rejected %d, returned:%d", *OriginalItemInfo.GetDebugString(), ItemInfoAdded.Amount, RejectedItemInfo.Amount, ReturnedItemInfo.Amount);
}

FGIS_ItemInfo UGIS_ItemCollection::RemoveInternal(const FGIS_ItemInfo& ItemInfo)
{
	int32 Removed = 0;

	FGIS_ItemStack ItemStackToRemove;

	{
		int32 Idx = Container.IndexOfByIds(ItemInfo.StackId, ItemInfo.Item->GetItemId());

		if (Idx != INDEX_NONE)
		{
			ItemStackToRemove = SimpleInternalItemRemove(ItemInfo, Removed, Idx);
		}
	}

	//如果已经移除的数量未达到指定移除的数量，就继续移除。
	if (Removed < ItemInfo.Amount)
	{
		TArray<FGIS_ItemStack> TempStacks = Container.Stacks;
		for (int32 i = TempStacks.Num() - 1; i >= 0; i--)
		{
			if (TempStacks[i].Item == nullptr || TempStacks[i].Item->GetItemId() != ItemInfo.Item->GetItemId())
			{
				continue;
			}
			ItemStackToRemove = SimpleInternalItemRemove(ItemInfo, Removed, i);
			if (Removed >= ItemInfo.Amount)
			{
				break;
			}
		}
	}

	const FGIS_ItemInfo RemovedItemInfo = FGIS_ItemInfo(ItemInfo.Item, Removed, this, ItemStackToRemove.Id);

	if (Removed == 0)
	{
		return RemovedItemInfo;
	}

	return RemovedItemInfo;
}

FGIS_ItemStack UGIS_ItemCollection::SimpleInternalItemRemove(const FGIS_ItemInfo& ItemInfo, int32& AlreadyRemoved, int32 StackIndex)
{
	check(Container.Stacks.IsValidIndex(StackIndex));
	const FGIS_ItemStack FoundStack = Container.Stacks[StackIndex];
	int32 RemainingToRemove = ItemInfo.Amount - AlreadyRemoved; //我要减10个，已经减了4个，还要减6个
	int32 NewAmount = FoundStack.Amount - RemainingToRemove;
	if (NewAmount <= 0) //如果栈里还有3个，不够减，那么已经减去的数量就成了7个，然后栈被移除。
	{
		AlreadyRemoved += FoundStack.Amount;
		RemoveItemStackAtIndex(StackIndex);
	}
	else //如果栈里还有7个，够减，那么已经减去的数量为10,栈还剩下7-6=1个。
	{
		AlreadyRemoved += RemainingToRemove;
		UpdateItemStackAmountAtIndex(StackIndex, NewAmount);
	}
	return FoundStack;
}

FGIS_ItemInfo UGIS_ItemCollection::GiveItem(const FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ItemCollection)
{
	if (OwningInventory->GetOwnerRole() != ROLE_Authority)
	{
		GIS_CLOG(Warning, "Has no authority")
		return FGIS_ItemInfo::None;
	}
	if (!ItemInfo.IsValid())
	{
		GIS_CLOG(Warning, "invalid ItemInfo to give.")
		return FGIS_ItemInfo::None;
	}

	if (ItemInfo.Item->GetOwningInventory() == ItemCollection->GetOwningInventory())
	{
		GIS_CLOG(Warning, "Item:%s already belongs to this inventory.", *ItemInfo.GetDebugString());
		return FGIS_ItemInfo::None;
	}

	FGIS_ItemInfo RemovedItemInfo = RemoveItem(ItemInfo);

	FGIS_ItemInfo ItemInfoToAdd = FGIS_ItemInfo(ItemInfo.Item, RemovedItemInfo.Amount, this);
	FGIS_ItemInfo GivenItemInfo = ItemCollection->AddItem(ItemInfoToAdd);
	if (GivenItemInfo.Amount != RemovedItemInfo.Amount)
	{
		// Failed to add so add it back to the previous collection.
	}
	return GivenItemInfo;
}

void UGIS_ItemCollection::ServerGiveItem_Implementation(const FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ItemCollection)
{
	GiveItem(ItemInfo, ItemCollection);
}

void UGIS_ItemCollection::GiveAllItems(UGIS_ItemCollection* OtherItemCollection)
{
	for (int i = Container.Stacks.Num() - 1; i >= 0; i--)
	{
		auto& itemStack = Container.Stacks[i];
		GiveItem(FGIS_ItemInfo(itemStack), OtherItemCollection);
	}
}

void UGIS_ItemCollection::ServerGiveAllItems_Implementation(UGIS_ItemCollection* OtherItemCollection)
{
	GiveAllItems(OtherItemCollection);
}

int32 UGIS_ItemCollection::GetItemAmountFittingInLimitedAdditionalStacks(const FGIS_ItemInfo& ItemInfo, int32 AvailableAdditionalStacks) const
{
	if (ItemInfo.Item->IsUnique())
	{
		//预计还有5个，而要添加10个，那么就只能5个。
		//预计还有10个，而要添加6个，那么就能放6个。
		return FMath::Min(ItemInfo.Amount, AvailableAdditionalStacks);
	}

	//满了，且没有已经存在的Stack可以叠上去。
	if (AvailableAdditionalStacks == 0 && !HasItem(ItemInfo.Item, 1))
	{
		return 0;
	}
	return ItemInfo.Amount;
}
