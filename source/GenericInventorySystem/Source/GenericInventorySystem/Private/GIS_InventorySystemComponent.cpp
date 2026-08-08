// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_InventorySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GIS_InventoryTags.h"
#include "GIS_InventorySubsystem.h"
#include "Items/GIS_ItemInstance.h"
#include "GIS_ItemCollection.h"
#include "Net/UnrealNetwork.h"
#include "GIS_ItemSlotCollection.h"
#include "GIS_CurrencySystemComponent.h"
#include "GIS_LogChannels.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_InventorySystemComponent)

UGIS_InventorySystemComponent::UGIS_InventorySystemComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), CollectionContainer(this)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	bWantsInitializeComponent = true;
}

bool UGIS_InventorySystemComponent::FindInventorySystemComponent(const AActor* Actor, UGIS_InventorySystemComponent*& Inventory)
{
	Inventory = GetInventorySystemComponent(Actor);

	return IsValid(Inventory);
}

UGIS_InventorySystemComponent* UGIS_InventorySystemComponent::GetInventorySystemComponent(const AActor* Actor)
{
	UGIS_InventorySystemComponent* Inventory = Actor ? Actor->FindComponentByClass<UGIS_InventorySystemComponent>() : nullptr;
	if (!IsValid(Inventory))
	{
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			if (APlayerState* PS = Cast<APlayerState>(Pawn->GetPlayerState()))
			{
				Inventory = PS->FindComponentByClass<UGIS_InventorySystemComponent>();
			}
		}
	}
	return Inventory;
}

UGIS_InventorySystemComponent* UGIS_InventorySystemComponent::FindInventorySystemComponent(const AActor* Actor)
{
	return GetInventorySystemComponent(Actor);
}

void UGIS_InventorySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bInventorySystemInitialized);
	DOREPLIFETIME(ThisClass, CollectionContainer);
}

bool UGIS_InventorySystemComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UGIS_InventorySystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ProcessPendingCollections();
}

bool UGIS_InventorySystemComponent::CanAddItem(const FGIS_ItemInfo& InItemInfo, FGIS_ItemInfo& OutItemInfo) const
{
	if (UGIS_ItemCollection* Collection = DetermineTargetCollection(InItemInfo))
	{
		return Collection->CanAddItem(InItemInfo, OutItemInfo);
	}
	return false;
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::DetermineTargetCollection(const FGIS_ItemInfo& ItemInfo) const
{
	UGIS_ItemCollection* Collection = nullptr;

	if (ItemInfo.CollectionId.IsValid())
	{
		Collection = GetCollectionById(ItemInfo.CollectionId);
	}
	if (Collection == nullptr && ItemInfo.CollectionTag.IsValid())
	{
		Collection = GetCollectionByTag(ItemInfo.CollectionTag);
	}
	if (Collection == nullptr)
	{
		Collection = GetDefaultCollection();
	}
	return Collection;
}

FGIS_ItemInfo UGIS_InventorySystemComponent::AddItem(const FGIS_ItemInfo& ItemInfo)
{
	if (UGIS_ItemCollection* Collection = DetermineTargetCollection(ItemInfo))
	{
		const FGIS_ItemInfo AddedItem = Collection->AddItem(ItemInfo);
		return AddedItem;
	}
	return FGIS_ItemInfo::None;
}

TArray<FGIS_ItemInfo> UGIS_InventorySystemComponent::AddItems(TArray<FGIS_ItemInfo> ItemInfos)
{
	TArray<FGIS_ItemInfo> AddedItemInfos;
	for (const FGIS_ItemInfo& ItemInfo : ItemInfos)
	{
		AddedItemInfos.Add(AddItem(ItemInfo));
	}
	return AddedItemInfos;
}

FGIS_ItemInfo UGIS_InventorySystemComponent::AddItemByDefinition(const FGameplayTag CollectionTag, TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, const int32 NewAmount)
{
	if (!GetOwner()->HasAuthority())
	{
		GIS_CLOG(Warning, "Has no authority!")
		return FGIS_ItemInfo::None;
	}
	if (UGIS_ItemInstance* NewItem = UGIS_InventorySubsystem::Get(GetWorld())->CreateItem(GetOwner(), ItemDefinition))
	{
		return AddItem(FGIS_ItemInfo(NewItem, NewAmount, CollectionTag));
	}
	return FGIS_ItemInfo::None;
}

void UGIS_InventorySystemComponent::ServerAddItemByDefinition_Implementation(const FGameplayTag CollectionTag, const TSoftObjectPtr<UGIS_ItemDefinition>& ItemDefinition, const int32 NewAmount)
{
	AddItemByDefinition(CollectionTag, ItemDefinition, NewAmount);
}

bool UGIS_InventorySystemComponent::CanMoveItem(const FGIS_ItemInfo& ItemInfo) const
{
	if (!ItemInfo.IsValid() || ItemInfo.Item->GetOwningCollection() == nullptr)
	{
		GIS_CLOG(Verbose, "item:%s has no source collection!", *ItemInfo.GetDebugString())
		return false;
	}

	UGIS_ItemCollection* SrcCollection = ItemInfo.Item->GetOwningCollection();

	if (ItemInfo.Item->GetOwningInventory() != this)
	{
		GIS_CLOG(Warning, "item:%s not belong to this inventory.", *ItemInfo.GetDebugString())
		return false;
	}

	UGIS_ItemCollection* TargetCollection = DetermineTargetCollection(ItemInfo);

	if (TargetCollection == nullptr || TargetCollection == SrcCollection)
	{
		GIS_CLOG(Warning, "no dest collection for item:%s to move", *ItemInfo.GetDebugString())
		return false;
	}
	if (TargetCollection == SrcCollection)
	{
		GIS_CLOG(Warning, "item:%s already in same collection.", *ItemInfo.GetDebugString())
		return false;
	}

	return true;
}

void UGIS_InventorySystemComponent::MoveItem(const FGIS_ItemInfo& ItemInfo)
{
	if (!GetOwner()->HasAuthority())
	{
		GIS_CLOG(Warning, "Has no authority!")
		return;
	}

	if (!CanMoveItem(ItemInfo))
	{
		return;
	}

	UGIS_ItemInstance* Item = ItemInfo.Item;
	UGIS_ItemCollection* SrcCollection = Item->GetOwningCollection();

	UGIS_ItemCollection* DestCollection = DetermineTargetCollection(ItemInfo);

	// This action used to give the item one way and then the other.
	// The action now removes the item, before it adds it to the other collection to allow restrictions to work properly
	FGIS_ItemInfo OriginalItem = SrcCollection->RemoveItem(ItemInfo);
	FGIS_ItemInfo MovedItemInfo = ItemInfo.None;

	if (UGIS_ItemSlotCollection* DestSlotCollection = Cast<UGIS_ItemSlotCollection>(DestCollection))
	{
		int32 slotIndex = ItemInfo.StackId.IsValid() ? DestSlotCollection->StackIdToSlotIndex(ItemInfo.StackId) : INDEX_NONE;

		if (slotIndex == INDEX_NONE) // fallback to suitable index.
		{
			slotIndex = DestSlotCollection->GetTargetSlotIndex(Item);
		}
		if (slotIndex != INDEX_NONE)
		{
			FGIS_ItemInfo previousItemInSlot = DestSlotCollection->GetItemInfoAtSlot(slotIndex);

			if (previousItemInSlot.Item != nullptr)
			{
				// If the previous item is stackable don't remove it.
				if (previousItemInSlot.Item->StackableEquivalentTo(OriginalItem.Item))
				{
					previousItemInSlot = ItemInfo.None;
				}
				else
				{
					previousItemInSlot = DestSlotCollection->RemoveItem(slotIndex);
				}
			}

			MovedItemInfo = DestSlotCollection->AddItem(OriginalItem, slotIndex);

			if (previousItemInSlot.Item != nullptr)
			{
				SrcCollection->AddItem(previousItemInSlot);
			}
		}
	}
	else
	{
		MovedItemInfo = DestCollection->AddItem(OriginalItem);
	}

	// Not all the item was added, return the items to the default collection.
	if (MovedItemInfo.Amount != OriginalItem.Amount)
	{
		int32 AmountToReturn = OriginalItem.Amount - MovedItemInfo.Amount;
		SrcCollection->AddItem(FGIS_ItemInfo(AmountToReturn, OriginalItem));
	}
}

void UGIS_InventorySystemComponent::ServerMoveItem_Implementation(const FGIS_ItemInfo& ItemInfo)
{
	MoveItem(ItemInfo);
}

bool UGIS_InventorySystemComponent::CanRemoveItem(const FGIS_ItemInfo& ItemInfo) const
{
	if (ItemInfo.IsValid())
	{
		FGIS_ItemInfo ItemInfoToRemove;
		return ItemInfo.Item->GetOwningCollection()->RemoveItemCondition(ItemInfo, ItemInfoToRemove);
	}
	return false;
}

FGIS_ItemInfo UGIS_InventorySystemComponent::RemoveItem(const FGIS_ItemInfo& ItemInfo)
{
	if (ItemInfo.Item->GetOwningCollection() != nullptr && ItemInfo.Item->GetOwningCollection()->GetOwningInventory() == this)
	{
		return ItemInfo.Item->GetOwningCollection()->RemoveItem(ItemInfo);
	}
	return DetermineTargetCollection(ItemInfo)->RemoveItem(ItemInfo);
}

void UGIS_InventorySystemComponent::ServerRemoveItem_Implementation(FGIS_ItemInfo ItemInfo)
{
	check(GetOwnerRole() == ROLE_Authority)
	RemoveItem(ItemInfo);
}

FGIS_ItemInfo UGIS_InventorySystemComponent::RemoveItemByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, const int32 Amount)
{
	if (ItemDefinition.IsNull() || Amount == 0)
	{
		return FGIS_ItemInfo::None;
	}

	// The item can be in multiple stacks, for example if it is Unique.

	int32 AmountRemoved = 0;
	int32 AmountToRemove = Amount;
	FGIS_ItemInfo LastItemInfoRemoved = FGIS_ItemInfo::None;

	for (int32 i = 0; i < Amount; i++)
	{
		FGIS_ItemInfo ItemInfo;
		if (!GetItemInfoByDefinition(ItemDefinition, ItemInfo))
		{
			break;
		}

		LastItemInfoRemoved = RemoveItem(ItemInfo);

		AmountRemoved += LastItemInfoRemoved.Amount;
		AmountToRemove = Amount - AmountRemoved;

		if (AmountToRemove == 0)
		{
			break;
		}
	}

	return FGIS_ItemInfo(AmountRemoved, LastItemInfoRemoved);
}

void UGIS_InventorySystemComponent::RemoveAllItems(bool RemoveItemsFromIgnoredCollections, bool DisableEventsWhileRemoving)
{
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		TObjectPtr<UGIS_ItemCollection> ItemCollection = CollectionContainer.Entries[i].Instance;
		if (RemoveItemsFromIgnoredCollections == false && IsIgnoredCollection(ItemCollection))
		{
			continue;
		}
		ItemCollection->RemoveAll();
	}
}

int32 UGIS_InventorySystemComponent::GetItemAmount(UGIS_ItemInstance* Item, bool SimilarItem) const
{
	if (Item == nullptr)
	{
		return 0;
	}

	int32 Amount = 0;

	for (int i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		if (IsIgnoredCollection(CollectionContainer.Entries[i].Instance))
		{
			continue;
		}

		Amount += CollectionContainer.Entries[i].Instance->GetItemAmount(Item);
	}

	return Amount;
}

int32 UGIS_InventorySystemComponent::GetItemAmountByDefinition(TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, bool Unique) const
{
	int32 Amount = 0;
	for (int i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		if (IsIgnoredCollection(CollectionContainer.Entries[i].Instance))
		{
			continue;
		}

		Amount += CollectionContainer.Entries[i].Instance->GetItemAmount(ItemDefinition, Unique);
	}

	return Amount;
}

bool UGIS_InventorySystemComponent::GetItemInfoInCollection(UGIS_ItemInstance* Item, const FGameplayTag CollectionTag, FGIS_ItemInfo& OutItemInfo) const
{
	if (UGIS_ItemCollection* Collection = GetCollectionByTag(CollectionTag))
	{
		return Collection->GetItemInfo(Item, OutItemInfo);
	}
	return false;
}

bool UGIS_InventorySystemComponent::FindItemInfoInCollection(UGIS_ItemInstance* Item, const FGameplayTag CollectionTag, FGIS_ItemInfo& OutItemInfo) const
{
	return GetItemInfoInCollection(Item, CollectionTag, OutItemInfo);
}

bool UGIS_InventorySystemComponent::GetAllItemInfosInCollection(const FGameplayTag CollectionTag, TArray<FGIS_ItemInfo>& OutItemInfos) const
{
	if (UGIS_ItemCollection* Collection = GetCollectionByTag(CollectionTag))
	{
		OutItemInfos = Collection->GetAllItemInfos();
		return OutItemInfos.Num() != 0;
	}
	return false;
}

bool UGIS_InventorySystemComponent::FindAllItemInfosInCollection(const FGameplayTag CollectionTag, TArray<FGIS_ItemInfo>& OutItemInfos) const
{
	return GetAllItemInfosInCollection(CollectionTag, OutItemInfos);
}

bool UGIS_InventorySystemComponent::GetItemInfo(UGIS_ItemInstance* Item, FGIS_ItemInfo& ItemInfo) const
{
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		if (IsIgnoredCollection(CollectionContainer.Entries[i].Instance))
		{
			continue;
		}
		if (CollectionContainer.Entries[i].Instance->GetItemInfo(Item, ItemInfo))
		{
			return true;
		}
	}
	return false;
}

bool UGIS_InventorySystemComponent::FindItemInfo(UGIS_ItemInstance* Item, FGIS_ItemInfo& ItemInfo) const
{
	return GetItemInfo(Item, ItemInfo);
}

bool UGIS_InventorySystemComponent::GetItemInfoByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, FGIS_ItemInfo& OutItemInfo) const
{
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		if (IsIgnoredCollection(CollectionContainer.Entries[i].Instance))
		{
			continue;
		}
		if (CollectionContainer.Entries[i].Instance->GetItemInfoByDefinition(ItemDefinition, OutItemInfo))
		{
			return OutItemInfo.IsValid();
		}
	}

	return false;
}

bool UGIS_InventorySystemComponent::FindItemInfoByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, FGIS_ItemInfo& OutItemInfo) const
{
	return GetItemInfoByDefinition(ItemDefinition, OutItemInfo);
}

bool UGIS_InventorySystemComponent::GetItemInfosByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, TArray<FGIS_ItemInfo>& OutItemInfos) const
{
	if (ItemDefinition.IsNull())
	{
		return false;
	}

	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		if (IsIgnoredCollection(CollectionContainer.Entries[i].Instance))
		{
			continue;
		}
		TArray<FGIS_ItemInfo> ItemInfos;
		if (CollectionContainer.Entries[i].Instance->GetItemInfosByDefinition(ItemDefinition, ItemInfos))
		{
			ItemInfos.Append(ItemInfos);
		}
	}

	return !OutItemInfos.IsEmpty();
}

bool UGIS_InventorySystemComponent::FindItemInfosByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, TArray<FGIS_ItemInfo>& OutItemInfos) const
{
	return GetItemInfosByDefinition(ItemDefinition, OutItemInfos);
}

TArray<FGIS_ItemInfo> UGIS_InventorySystemComponent::GetItemInfos() const
{
	TArray<FGIS_ItemInfo> Ret;
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		if (IsIgnoredCollection(CollectionContainer.Entries[i].Instance))
		{
			continue;
		}

		TArray<FGIS_ItemInfo> AllItemInfos = CollectionContainer.Entries[i].Instance->GetAllItemInfos();
		Ret.Append(AllItemInfos);
	}
	return Ret;
}

bool UGIS_InventorySystemComponent::HasEnoughItem(const TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, int32 Amount) const
{
	TArray<FGIS_ItemInfo> OutItemInfos;
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		UGIS_ItemCollection* Collection = CollectionContainer.Entries[i].Instance;
		TArray<FGIS_ItemInfo> OutItemInfosTemp;
		Collection->GetItemInfosByDefinition(ItemDefinition, OutItemInfosTemp);
		OutItemInfos.Append(OutItemInfosTemp);
	}
	if (OutItemInfos.Num() >= Amount)
	{
		return true;
	}
	for (const auto& It : OutItemInfos)
	{
		if (It.Amount >= Amount)
		{
			return true;
		}
	}
	return false;
}

TArray<UGIS_ItemCollection*> UGIS_InventorySystemComponent::GetItemCollections() const
{
	TArray<UGIS_ItemCollection*> Ret;
	Ret.Reserve(CollectionContainer.Entries.Num());
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		Ret.Add(CollectionContainer.Entries[i].Instance);
	}
	return Ret;
}

bool UGIS_InventorySystemComponent::IsDefaultCollectionCreated() const
{
	for (auto& Definition : CollectionDefinitions)
	{
		bool bFound = false;
		for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
		{
			const FGIS_CollectionEntry& Entry = CollectionContainer.Entries[i];
			if (Entry.IsValidEntry() && Entry.Definition == Definition && Entry.Instance->GetCollectionTag() == Definition->CollectionTag)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			return false;
		}
	}
	return true;
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::GetDefaultCollection() const
{
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		auto& Instance = CollectionContainer.Entries[i].Instance;
		if (Instance && Instance->GetCollectionTag().MatchesTagExact(GIS_CollectionTags::Main))
		{
			return Instance;
		}
	}
	return nullptr;
}

int32 UGIS_InventorySystemComponent::GetCollectionCount() const
{
	return CollectionContainer.Entries.Num();
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::GetCollectionByTag(const FGameplayTag CollectionTag) const
{
	if (!CollectionTag.IsValid())
	{
		return nullptr;
	}
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		auto& Instance = CollectionContainer.Entries[i].Instance;

		if (Instance && Instance->GetCollectionTag() == CollectionTag)
		{
			return Instance;
		}
	}
	return nullptr;
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::GetCollectionByTags(FGameplayTagContainer Tags)
{
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		if (CollectionContainer.Entries[i].Instance && CollectionContainer.Entries[i].Instance->GetCollectionTag().MatchesAnyExact(Tags))
		{
			return CollectionContainer.Entries[i].Instance;
		}
	}
	return nullptr;
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::GetCollectionById(FGuid CollectionId) const
{
	if (CollectionIdToInstanceMap.Contains(CollectionId))
	{
		return CollectionIdToInstanceMap[CollectionId];
	}
	return nullptr;
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::GetTypedCollectionByTag(const FGameplayTag CollectionTag, TSubclassOf<UGIS_ItemCollection> DesiredClass) const
{
	if (UClass* RealClass = DesiredClass)
	{
		if (UGIS_ItemCollection* Collection = GetCollectionByTag(CollectionTag))
		{
			if (Collection->GetClass() == RealClass)
			{
				return Collection;
			}
		}
	}
	return nullptr;
}

bool UGIS_InventorySystemComponent::FindTypedCollectionByTag(const FGameplayTag CollectionTag, TSubclassOf<UGIS_ItemCollection> DesiredClass, UGIS_ItemCollection*& OutCollection)
{
	if (UClass* RealClass = DesiredClass)
	{
		if (UGIS_ItemCollection* Collection = GetCollectionByTag(CollectionTag))
		{
			if (Collection->GetClass() == RealClass)
			{
				OutCollection = Collection;
				return true;
			}
		}
	}
	return false;
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::AddCollectionByDefinition(TSoftObjectPtr<const UGIS_ItemCollectionDefinition> CollectionDefinition)
{
	if (!GetOwner()->HasAuthority())
	{
		GIS_CLOG(Warning, "Has no authority!")
		return nullptr;
	}

	if (CollectionDefinition.IsNull())
	{
		GIS_CLOG(Warning, "Try to add collection with invalid definition.")
		return nullptr;
	}

	const UGIS_ItemCollectionDefinition* Definition = CollectionDefinition.LoadSynchronous();
	if (!IsValid(Definition))
	{
		GIS_CLOG(Warning, "Try to add collection with invalid definition.")
		return nullptr;
	}

	UGIS_ItemCollection* NewCollection = CreateCollectionInstance(Definition);

	check(NewCollection)

	FGIS_CollectionEntry NewEntry;
	NewEntry.Id = FGuid::NewGuid();
	NewEntry.Instance = NewCollection;
	NewEntry.Definition = Definition;
	if (AddCollectionEntry(NewEntry))
	{
		return NewEntry.Instance;
	}
	return nullptr;
}

bool UGIS_InventorySystemComponent::AddCollectionEntry(const FGIS_CollectionEntry& NewEntry)
{
	if (!NewEntry.IsValidEntry())
	{
		return false;
	}
	if (NewEntry.Instance && NewEntry.Instance->IsInitialized())
	{
		GIS_CLOG(Warning, "Try to add already initialized collection.")
		return false;
	}

	FGIS_CollectionEntry& AddedEntry = CollectionContainer.Entries.Add_GetRef(NewEntry);
	if (GetOwnerRole() >= ROLE_Authority && IsReadyForReplication())
	{
		if (!IsReplicatedSubObjectRegistered(AddedEntry.Instance))
		{
			AddReplicatedSubObject(AddedEntry.Instance);
		}
	}
	OnCollectionAdded(NewEntry);
	CollectionContainer.MarkItemDirty(AddedEntry);
	GetOwner()->ForceNetUpdate();
	return true;
}

void UGIS_InventorySystemComponent::RemoveCollectionEntry(int32 Idx)
{
	if (CollectionContainer.Entries.IsValidIndex(Idx))
	{
		const FGIS_CollectionEntry& Entry = CollectionContainer.Entries[Idx];
		if (IsValid(Entry.Instance) && Entry.Instance->IsInitialized())
		{
			OnCollectionRemoved(Entry);
			RemoveReplicatedSubObject(Entry.Instance);
			CollectionContainer.Entries.RemoveAt(Idx);
			CollectionContainer.MarkArrayDirty();
		}
	}
}

UGIS_ItemCollection* UGIS_InventorySystemComponent::CreateCollectionInstance(const UGIS_ItemCollectionDefinition* CollectionDefinition)
{
	if (!IsValid(CollectionDefinition))
	{
		GIS_CLOG(Warning, "Try to add collection with invalid definition.")
		return nullptr;
	}
	TSubclassOf<UGIS_ItemCollection> CollectionClass = CollectionDefinition->GetCollectionInstanceClass();
	if (CollectionClass == nullptr)
	{
		GIS_CLOG(Warning, "definition(%s) doesn't specify valid item collection class.", *CollectionDefinition->GetName())
		return nullptr;
	}
	UGIS_ItemCollection* NewCollection = NewObject<UGIS_ItemCollection>(GetOwner(), CollectionClass);
	if (NewCollection == nullptr)
	{
		GIS_CLOG(Error, "failed to create instance of %s", *GetNameSafe(CollectionDefinition))
		return nullptr;
	}
	return NewCollection;
}

UGIS_CurrencySystemComponent* UGIS_InventorySystemComponent::GetCurrencySystem() const
{
	return CurrencySystem;
}


void UGIS_InventorySystemComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing Item Collections.
	if (IsUsingRegisteredSubObjectList())
	{
		for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
		{
			auto& Instance = CollectionContainer.Entries[i].Instance;
			if (IsValid(Instance))
			{
				if (!IsReplicatedSubObjectRegistered(Instance))
				{
					AddReplicatedSubObject(Instance);
				}
				for (const FGIS_ItemStack& ItemStack : Instance->GetAllItemStacks())
				{
					if (ItemStack.Item == nullptr)
					{
						continue;
					}
					if (!IsReplicatedSubObjectRegistered(ItemStack.Item))
					{
						AddReplicatedSubObject(ItemStack.Item);
					}
				}
			}
		}
	}
}

void UGIS_InventorySystemComponent::LoadDefaultLoadouts()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	for (int32 i = 0; i < DefaultLoadouts.Num(); i++)
	{
		const FGIS_DefaultLoadout& Loadout = DefaultLoadouts[i];
		if (!Loadout.Tag.IsValid())
		{
			continue;
		}
		for (const FGIS_ItemDefinitionAmount& DefaultItem : Loadout.DefaultItems)
		{
			if (DefaultItem.Amount <= 0)
			{
				continue;
			}

			if (DefaultItem.Definition.IsNull())
			{
				continue;
			}
			UGIS_ItemInstance* Item = UGIS_InventorySubsystem::Get(GetWorld())->CreateItem(Cast<AActor>(GetOuter()), DefaultItem.Definition);
			if (Item == nullptr)
			{
				GIS_CLOG(Warning, "Failed to add DefaultLoadout{Definition:%s}", *DefaultItem.Definition.ToString())
				continue;
			}
			FGIS_ItemInfo Info;
			Info.Item = Item;
			Info.Amount = DefaultItem.Amount;
			Info.CollectionTag = Loadout.Tag;
			AddItem(Info);
		}
	}
}

void UGIS_InventorySystemComponent::ServerLoadDefaultLoadouts_Implementation()
{
	ServerLoadDefaultLoadouts();
}

void UGIS_InventorySystemComponent::OnInventorySystemInitialized_Implementation()
{
	OnInventorySystemInitializedEvent.Broadcast();

	TArray<FGIS_InventorySystem_Initialized_DynamicEvent> Delegates = InitializedDelegates;
	for (FGIS_InventorySystem_Initialized_DynamicEvent Delegate : Delegates)
	{
		Delegate.ExecuteIfBound();
	}
	InitializedDelegates.Empty();
}

void UGIS_InventorySystemComponent::InitializeInventorySystem()
{
	if (bInventorySystemInitialized || !GetOwner()->HasAuthority())
	{
		GIS_CLOG(Verbose, "already initialized or has no authority!");
		return;
	}
	CollectionIdToInstanceMap.Empty();
	PendingCollections.Empty();
	for (int32 i = 0; i < CollectionDefinitions.Num(); i++)
	{
		if (CollectionDefinitions[i] == nullptr || !CollectionDefinitions[i]->CollectionTag.IsValid())
		{
			GIS_CLOG(Error, "default collection definition at index(%d) is invalid or mising collection tag!", i);
			continue;
		}
		if (AddCollectionByDefinition(CollectionDefinitions[i]) == nullptr)
		{
			GIS_CLOG(Warning, "failed to initialize collection:%s", *GetNameSafe(CollectionDefinitions[i]))
		}
	}
	LoadDefaultLoadouts();

	bInventorySystemInitialized = true;
	OnInventorySystemInitialized();
}

void UGIS_InventorySystemComponent::InitializeInventorySystemWithRecord(const FGIS_InventoryRecord& InventoryRecord)
{
	if (bInventorySystemInitialized || !GetOwner()->HasAuthority())
	{
		GIS_CLOG(Verbose, "already initialized or has no authority!");
		return;
	}

	if (!InventoryRecord.IsValid())
	{
		GIS_CLOG(Verbose, "provided invalid records");
		return;
	}

	UGIS_InventorySubsystem* Subsystem = UGIS_InventorySubsystem::Get(GetWorld());
	if (Subsystem == nullptr)
	{
		GIS_CLOG(Verbose, "missing inventory sub system");
		return;
	}

	CollectionIdToInstanceMap.Empty();
	PendingCollections.Empty();

	ResetInventorySystem();

	Subsystem->DeserializeInventory(this, InventoryRecord);

	bInventorySystemInitialized = true;
	OnInventorySystemInitialized();
}

void UGIS_InventorySystemComponent::ResetInventorySystem()
{
	if (!bInventorySystemInitialized || !GetOwner()->HasAuthority())
	{
		GIS_CLOG(Verbose, "not initialized or has no authority!");
		return;
	}

	//remove all items inside collections.
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		auto& Instance = CollectionContainer.Entries[i].Instance;
		if (IsValid(Instance) && Instance->IsInitialized())
		{
			Instance->RemoveAll();
		}
	}

	//remove all remaining collection itself.
	for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
	{
		RemoveCollectionEntry(i);
	}

	bInventorySystemInitialized = false;
}

bool UGIS_InventorySystemComponent::IsInventoryInitialized() const
{
	return bInventorySystemInitialized;
}

void UGIS_InventorySystemComponent::BindToInventorySystemInitialized(FGIS_InventorySystem_Initialized_DynamicEvent Delegate)
{
	if (bInventorySystemInitialized)
	{
		Delegate.ExecuteIfBound();
	}
	else
	{
		InitializedDelegates.Add(Delegate);
	}
}

void UGIS_InventorySystemComponent::OnRegister()
{
	Super::OnRegister();
}

void UGIS_InventorySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		return;
	}

	CollectionContainer.OwningComponent = this;

	if (!GetOwner()->IsUsingRegisteredSubObjectList())
	{
		GIS_CLOG(Error, "requires enable bReplicateUsingRegisteredSubObjectList.")
	}

	CurrencySystem = UGIS_CurrencySystemComponent::GetCurrencySystemComponent(GetOwner());
}

// Called when the game starts
void UGIS_InventorySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bInitializeOnBeginplay && GetOwner()->HasAuthority())
	{
		InitializeInventorySystem();
	}
}

bool UGIS_InventorySystemComponent::IsIgnoredCollection(UGIS_ItemCollection* ItemCollection) const
{
	if (ItemCollection == nullptr)
	{
		return true;
	}
	return IgnoredCollections.HasTagExact(ItemCollection->GetCollectionTag());
}

void UGIS_InventorySystemComponent::OnCollectionAdded(const FGIS_CollectionEntry& Entry)
{
	check(IsValid(Entry.Instance) && Entry.Id.IsValid())
	CollectionIdToInstanceMap.Add(Entry.Id, Entry.Instance);
	Entry.Instance->SetInventory(this);
	Entry.Instance->SetCollectionId(Entry.Id);
	Entry.Instance->SetCollectionTag(Entry.Definition->CollectionTag);
	Entry.Instance->SetDefinition(Entry.Definition);
	OnCollectionAddedEvent.Broadcast(Entry.Instance);
	GIS_CLOG(Verbose, "added collection:%s", *GetNameSafe(Entry.Instance->GetDefinition()))
}

void UGIS_InventorySystemComponent::OnCollectionRemoved(const FGIS_CollectionEntry& Entry)
{
	check(IsValid(Entry.Instance) && Entry.Id.IsValid())

	OnCollectionRemovedEvent.Broadcast(Entry.Instance);
	// clear collection data.
	Entry.Instance->SetInventory(nullptr);
	// remove cache.
	if (CollectionIdToInstanceMap.Contains(Entry.Id))
	{
		CollectionIdToInstanceMap.Remove(Entry.Id);
	}
	Entry.Instance->PendingItemStacks.Empty();
	GIS_CLOG(Verbose, "removed collection:%s", *GetNameSafe(Entry.Instance->GetDefinition()))
}

void UGIS_InventorySystemComponent::OnCollectionUpdated(const FGIS_CollectionEntry& Entry)
{
}

void UGIS_InventorySystemComponent::ProcessPendingCollections()
{
	if (HasBegunPlay() && GetOwner() != nullptr)
	{
		TArray<FGuid> Added;
		for (const auto& Pending : PendingCollections)
		{
			if (Pending.Value.IsValidEntry())
			{
				Added.AddUnique(Pending.Key);
			}
		}

		for (int32 i = 0; i < Added.Num(); i++)
		{
			FGuid AddedIndex = Added[i];
			const FGIS_CollectionEntry& AddedEntry = PendingCollections[AddedIndex];
			OnCollectionAdded(AddedEntry);
			GIS_CLOG(Verbose, "added collection:%s from pending list.", *GetNameSafe(AddedEntry.Instance->GetDefinition()))
			PendingCollections.Remove(AddedIndex);
		}

		for (int32 i = 0; i < CollectionContainer.Entries.Num(); i++)
		{
			UGIS_ItemCollection* Collection = CollectionContainer.Entries[i].Instance;
			if (IsValid(Collection) && Collection->IsInitialized())
			{
				Collection->ProcessPendingItemStacks();
			}
		}
	}
}

#if WITH_EDITOR

void UGIS_InventorySystemComponent::PostLoad()
{
	Super::PostLoad();
}

void UGIS_InventorySystemComponent::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}

EDataValidationResult UGIS_InventorySystemComponent::IsDataValid(FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif
