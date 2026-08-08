// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_EquipmentSystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GIS_EquipItemInstance.h"
#include "GIS_EquipmentInstance.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "GIS_ItemDefinition.h"
#include "GIS_ItemFragment_Equippable.h"
#include "GIS_ItemInstance.h"
#include "GIS_ItemSlotCollection.h"
#include "GIS_LogChannels.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_EquipmentSystemComponent)


UGIS_EquipmentSystemComponent::UGIS_EquipmentSystemComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), Container(this)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	bWantsInitializeComponent = true;
}

void UGIS_EquipmentSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Container);
	DOREPLIFETIME(ThisClass, bEquipmentSystemInitialized);
	DOREPLIFETIME_CONDITION(ThisClass, TargetCollectionDefinition, COND_OwnerOnly);
}

void UGIS_EquipmentSystemComponent::EquipItemToSlot(UGIS_ItemInstance* Item, const FGameplayTag& SlotTag)
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}

	if (!SlotTag.IsValid())
	{
		return;
	}

	UObject* EquipmentInstance = CreateEquipmentInstance(GetOwner(), Item);
	if (!IsValid(EquipmentInstance))
	{
		return;
	}

	FGIS_EquipmentEntry NewEntry;
	NewEntry.EquippedSlot = SlotTag;
	NewEntry.Instance = EquipmentInstance;
	NewEntry.ItemInstance = Item;
	NewEntry.bActive = false;
	NewEntry.bPrevActive = false;

	//Auto activation.
	if (auto Equippable = NewEntry.ItemInstance->FindFragmentByClass<UGIS_ItemFragment_Equippable>())
	{
		FGameplayTag MatchingGroup;
		int32 MatchingIdxInGroup;
		if (Equippable->bAutoActivate && CanActiveSlotInGroup(NewEntry.EquippedSlot, MatchingGroup, MatchingIdxInGroup))
		{
			NewEntry.bActive = true;
			GroupActiveIdxMap[MatchingGroup] = MatchingIdxInGroup;
		}
	}

	AddEquipmentEntry(NewEntry);
}

void UGIS_EquipmentSystemComponent::UnequipBySlot(FGameplayTag SlotTag)
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}
	int32 Idx = Container.IndexOfBySlot(SlotTag);
	if (Idx != INDEX_NONE)
	{
		RemoveEquipmentEntry(Idx);
	}
}

void UGIS_EquipmentSystemComponent::UnequipByItem(const FGuid& ItemId)
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}
	int32 Idx = Container.IndexOfByItemId(ItemId);
	if (Idx != INDEX_NONE)
	{
		RemoveEquipmentEntry(Idx);
	}
}

bool UGIS_EquipmentSystemComponent::OwnerHasAuthority() const
{
	AActor* Owner = GetOwner();
	return IsValid(Owner) && Owner->HasAuthority();
}

// void UGIS_EquipmentSystemComponent::UnequipInstance(UObject* EquipmentInstance)
// {
// 	for (int32 i = 0; i < Container.Entries.Num(); i++)
// 	{
// 		const FGIS_EquipmentEntry& Entry = Container.Entries[i];
// 		if (Entry.Instance == EquipmentInstance)
// 		{
// 			RemoveEquipmentEntry(i);
// 		}
// 	}
// }

TArray<UObject*> UGIS_EquipmentSystemComponent::GetEquipments(TSubclassOf<UObject> InstanceType, FGameplayTagQuery SlotQuery) const
{
	TArray<UObject*> Results;
	if (SlotQuery.IsEmpty())
	{
		return Results;
	}

	if (UClass* RealClass = InstanceType)
	{
		const TArray<FGIS_EquipmentEntry>& MatchedEntries = Container.Entries.FilterByPredicate([&SlotQuery,&RealClass](const FGIS_EquipmentEntry& Entry)
		{
			return Entry.Instance->IsA(RealClass) && SlotQuery.Matches(Entry.EquippedSlot.GetSingleTagContainer());
		});
		for (const FGIS_EquipmentEntry& Entry : MatchedEntries)
		{
			Results.AddUnique(Entry.Instance);
		}
	}
	return Results;
}

TArray<UObject*> UGIS_EquipmentSystemComponent::GetActiveEquipments(TSubclassOf<UObject> InstanceType, FGameplayTagQuery SlotQuery) const
{
	TArray<UObject*> Results;
	if (SlotQuery.IsEmpty())
	{
		return Results;
	}

	if (UClass* RealClass = InstanceType)
	{
		const TArray<FGIS_EquipmentEntry>& MatchedEntries = Container.Entries.FilterByPredicate([&SlotQuery,&RealClass](const FGIS_EquipmentEntry& Entry)
		{
			return Entry.bActive && Entry.Instance->IsA(RealClass) && SlotQuery.Matches(Entry.EquippedSlot.GetSingleTagContainer());
		});
		for (const FGIS_EquipmentEntry& Entry : MatchedEntries)
		{
			Results.AddUnique(Entry.Instance);
		}
	}
	return Results;
}

UObject* UGIS_EquipmentSystemComponent::GetEquipment(TSubclassOf<UObject> InstanceType, FGameplayTagQuery SlotQuery) const
{
	if (UClass* RealClass = InstanceType)
	{
		const FGIS_EquipmentEntry* Found = Container.Entries.FindByPredicate([&SlotQuery,&RealClass](const FGIS_EquipmentEntry& Entry)
		{
			return Entry.Instance->IsA(RealClass) && SlotQuery.Matches(Entry.EquippedSlot.GetSingleTagContainer());
		});
		if (Found)
		{
			return Found->Instance;
		}
	}
	return nullptr;
}

UObject* UGIS_EquipmentSystemComponent::GetActiveEquipment(TSubclassOf<UObject> InstanceType, FGameplayTagQuery SlotQuery) const
{
	if (UClass* RealClass = InstanceType)
	{
		const FGIS_EquipmentEntry* Found = Container.Entries.FindByPredicate([&SlotQuery,&RealClass](const FGIS_EquipmentEntry& Entry)
		{
			return Entry.bActive && Entry.Instance->IsA(RealClass) && SlotQuery.Matches(Entry.EquippedSlot.GetSingleTagContainer());
		});
		if (Found)
		{
			return Found->Instance;
		}
	}
	return nullptr;
}

UGIS_EquipmentInstance* UGIS_EquipmentSystemComponent::GetEquipmentInstanceOfActor(AActor* EquipmentActor) const
{
	if (IsValid(EquipmentActor))
	{
		for (const FGIS_EquipmentEntry& Entry : Container.Entries)
		{
			if (UGIS_EquipmentInstance* Instance = Cast<UGIS_EquipmentInstance>(Entry.Instance))
			{
				if (Instance->GetEquipmentActors().Contains(EquipmentActor))
				{
					return Instance;
				}
			}
		}
	}
	return nullptr;
}

UGIS_EquipmentInstance* UGIS_EquipmentSystemComponent::GetTypedEquipmentInstanceOfActor(TSubclassOf<UGIS_EquipmentInstance> InstanceType, AActor* EquipmentActor) const
{
	if (UClass* RealClass = InstanceType)
	{
		if (UGIS_EquipmentInstance* Instance = GetEquipmentInstanceOfActor(EquipmentActor))
		{
			if (Instance->GetClass()->IsChildOf(RealClass))
			{
				return Instance;
			}
		}
	}
	return nullptr;
}

bool UGIS_EquipmentSystemComponent::IsSlotEquipped(FGameplayTag SlotTag) const
{
	return SlotToIdxMap.Contains(SlotTag);
}

int32 UGIS_EquipmentSystemComponent::SlotTagToEquipmentInex(FGameplayTag InSlotTag) const
{
	return Container.IndexOfBySlot(InSlotTag);
}

int32 UGIS_EquipmentSystemComponent::ItemIdToEquipmentInex(FGuid InItemId) const
{
	return Container.IndexOfByItemId(InItemId);
}

UObject* UGIS_EquipmentSystemComponent::GetEquipmentInSlot(FGameplayTag SlotTag) const
{
	int32 Idx = Container.IndexOfBySlot(SlotTag);

	if (Idx != INDEX_NONE)
	{
		return Container.Entries[Idx].Instance;
	}
	return nullptr;
}

UObject* UGIS_EquipmentSystemComponent::GetEquipmentByItem(const UGIS_ItemInstance* Item)
{
	if (Item == nullptr)
	{
		return nullptr;
	}
	int32 Idx = Container.IndexOfByItem(Item);
	if (Idx != INDEX_NONE)
	{
		return Container.Entries[Idx].Instance;
	}
	return nullptr;
}

void UGIS_EquipmentSystemComponent::SetEquipmentActiveState(FGameplayTag SlotTag, bool NewActiveState)
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}
	int32 Idx = Container.IndexOfBySlot(SlotTag);
	if (Idx != INDEX_NONE)
	{
		SetEquipmentActiveStateWithGroupRestriction(Idx, NewActiveState);
	}
}

void UGIS_EquipmentSystemComponent::ServerSetEquipmentActiveState(FGameplayTag SlotTag, bool NewActiveState)
{
	SetEquipmentActiveState(SlotTag, NewActiveState);
}

bool UGIS_EquipmentSystemComponent::CanActiveSlotInGroup(FGameplayTag SlotTag, FGameplayTag& MatchingGroup, int32& MatchingIdxInGroup)
{
	for (auto& Pair : GroupActiveIdxMap)
	{
		if (SlotTag.MatchesTag(Pair.Key))
		{
			MatchingGroup = Pair.Key;
			break;
		}
	}

	if (!MatchingGroup.IsValid())
	{
		return true;
	}
	// with in group but already has active equipment. Already occupied
	if (GroupActiveIdxMap[MatchingGroup] != INDEX_NONE)
	{
		return false;
	}
	MatchingIdxInGroup = TargetCollectionDefinition->GetSlotIndexWithinGroup(MatchingGroup, SlotTag);
	check(MatchingIdxInGroup != INDEX_NONE);
	return true;
}

void UGIS_EquipmentSystemComponent::SetEquipmentActiveStateWithGroupRestriction(int32 Idx, bool NewActiveState)
{
	if (!Container.Entries.IsValidIndex(Idx))
	{
		return;
	}

	const FGIS_EquipmentEntry& Entry = Container.Entries[Idx];
	if (Entry.bActive)
	{
		return;
	}

	FGameplayTag MatchingGroup = FGameplayTag::EmptyTag;
	for (auto& Pair : GroupActiveIdxMap)
	{
		if (Entry.EquippedSlot.MatchesTag(Pair.Key))
		{
			MatchingGroup = Pair.Key;
			break;
		}
	}


	if (MatchingGroup.IsValid())
	{
		// with in group but already has active equipment. Already occupied
		if (NewActiveState && GroupActiveIdxMap[MatchingGroup] != INDEX_NONE)
		{
			return;
		}
		if (NewActiveState)
		{
			int32 NewIdxInGroup = TargetCollectionDefinition->GetSlotIndexWithinGroup(MatchingGroup, Entry.EquippedSlot);
			check(NewIdxInGroup != INDEX_NONE);
			GroupActiveIdxMap[MatchingGroup] = NewIdxInGroup;
		}
		else
		{
			GroupActiveIdxMap[MatchingGroup] = INDEX_NONE;
		}
	}

	SetEquipmentActiveState(Idx, NewActiveState);
}

void UGIS_EquipmentSystemComponent::SetEquipmentActiveState(int32 Idx, bool NewActiveState)
{
	check(Container.Entries.IsValidIndex(Idx))
	if (Container.Entries[Idx].bActive == NewActiveState)
	{
		// Same state, return.
		return;
	}

	FGIS_EquipmentEntry& Entry = Container.Entries[Idx];

	Entry.bActive = NewActiveState;
	OnEquipmentEntryChanged(Entry, Idx);
	Container.MarkItemDirty(Entry);
}


void UGIS_EquipmentSystemComponent::OnTargetCollectionChanged(const FGIS_InventoryStackUpdateMessage& Message)
{
	// only handle equip/unequip on the server side.
	if (!OwnerHasAuthority())
	{
		return;
	}

	// Make use this message comes from the inventory&&collection I care about.
	bool bIsMyConcern = IsValid(Message.Inventory) && Message.Inventory == Inventory && Message.CollectionId == TargetCollection->GetCollectionId();

	if (!bIsMyConcern)
	{
		return;
	}

	switch (Message.ChangeType)
	{
	case EGIS_ItemStackChangeType::WasAdded:
		{
			FGameplayTag SlotTag = TargetCollection->GetItemSlotName(Message.Instance);
			if (!SlotTag.IsValid())
			{
				return;
			}
			EquipItemToSlot(Message.Instance, SlotTag);
			return;
		}
	case EGIS_ItemStackChangeType::WasRemoved:
		{
			UnequipByItem(Message.Instance->GetItemId());
		}
	default: break;
	}
}

void UGIS_EquipmentSystemComponent::ProcessPendingEquipments()
{
	if (HasBegunPlay() && GetOwner() != nullptr)
	{
		TArray<int32> AddedEquipments;
		for (auto& Pair : PendingEquipmentEntries)
		{
			if (Pair.Value.IsValid())
			{
				AddedEquipments.Add(Pair.Key);
			}
		}
		for (int32 i = 0; i < AddedEquipments.Num(); i++)
		{
			int32 idx = AddedEquipments[i];
			const FGIS_EquipmentEntry& Entry = PendingEquipmentEntries[idx];
			OnEquipmentEntryAdded(Entry, idx);
			PendingEquipmentEntries.Remove(idx);
		}
	}
}

void UGIS_EquipmentSystemComponent::OnEquipmentSystemInitialized_Implementation()
{
	OnEquipmentSystemInitializedEvent.Broadcast();

	TArray<FGIS_EquipmentSystem_Initialized_DynamicEvent> Delegates = InitializedDelegates;
	for (FGIS_EquipmentSystem_Initialized_DynamicEvent Delegate : Delegates)
	{
		Delegate.ExecuteIfBound();
	}
	InitializedDelegates.Empty();
}

void UGIS_EquipmentSystemComponent::OnTargetCollectionRemoved(UGIS_ItemCollection* Collection)
{
	if (Collection && TargetCollection && TargetCollection == Collection)
	{
		ResetEquipmentSystem();
	}
}

bool UGIS_EquipmentSystemComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FGIS_EquipmentEntry& Entry : Container.Entries)
	{
		if (IsValid(Entry.Instance))
		{
			if (IGIS_EquipmentInterface::Execute_IsReplicationManaged(Entry.Instance))
			{
				WroteSomething |= Channel->ReplicateSubobject(Entry.Instance, *Bunch, *RepFlags);
			}
		}
	}

	return WroteSomething;
}

void UGIS_EquipmentSystemComponent::OnRegister()
{
	Super::OnRegister();
}

void UGIS_EquipmentSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		return;
	}

	Container.OwningComponent = this;
	if (!GetOwner()->IsUsingRegisteredSubObjectList())
	{
		GIS_CLOG(Error, "requires enable bReplicateUsingRegisteredSubObjectList.")
	}
}

void UGIS_EquipmentSystemComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	// Register existing Equipment Instance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const TObjectPtr<UObject>& PendingObject : PendingReplicatedEquipments)
		{
			if (!IsReplicatedSubObjectRegistered(PendingObject))
			{
				AddReplicatedSubObject(PendingObject);
			}
		}
		PendingReplicatedEquipments.Empty();
	}
}

// Called when the game starts
void UGIS_EquipmentSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bInitializeOnBeginPlay && OwnerHasAuthority())
	{
		InitializeEquipmentSystem();
	}
}


void UGIS_EquipmentSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ProcessPendingEquipments();
}

void UGIS_EquipmentSystemComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void UGIS_EquipmentSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bInitializeOnBeginPlay)
	{
		ResetEquipmentSystem();
	}

	Super::EndPlay(EndPlayReason);
}


void UGIS_EquipmentSystemComponent::InitializeEquipmentSystem()
{
	if (bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "already initialized or has no authority!")
		return;
	}

	UGIS_InventorySystemComponent* InventorySystem = UGIS_InventorySystemComponent::FindInventorySystemComponent(GetOwner());

	if (InventorySystem == nullptr)
	{
		InventorySystem = UGIS_InventorySystemComponent::FindInventorySystemComponent(GetController<AController>());
	}

	if (!InventorySystem)
	{
		GIS_CLOG(Error, "doesn't have valid inventory system component!")
		return;
	}

	InitializeEquipmentSystemWithInventory(InventorySystem);
}

void UGIS_EquipmentSystemComponent::InitializeEquipmentSystemWithInventory(UGIS_InventorySystemComponent* InventorySystem)
{
	if (bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "already initialized or has no authority!")
		return;
	}

	if (!IsValid(InventorySystem))
	{
		GIS_CLOG(Error, "the inventory is invalid!")
		return;
	}

	if (!InventorySystem->IsInventoryInitialized())
	{
		GIS_CLOG(Error, "the inventory is not initialized!")
		return;
	}

	if (!TargetCollectionTag.IsValid())
	{
		GIS_CLOG(Error, "doesn't have valid target collection tag!")
		return;
	}

	UGIS_ItemSlotCollection* Collection = Cast<UGIS_ItemSlotCollection>(InventorySystem->GetCollectionByTag(GetTargetCollectionTag()));

	if (Collection == nullptr)
	{
		GIS_CLOG(Error, "%s's inventory doesn't have valid item slot collection with name:%s", *InventorySystem->GetOwner()->GetName(), *TargetCollectionTag.ToString());
		return;
	}
	Inventory = InventorySystem;
	TargetCollection = Collection;
	TargetCollectionDefinition = Collection->GetMyDefinition();

	GroupActiveIdxMap.Empty();
	for (const TPair<FGameplayTag, FGIS_ItemSlotGroup>& Pair : TargetCollectionDefinition->SlotGroupMap)
	{
		GroupActiveIdxMap.Add(Pair.Key, INDEX_NONE);
	}

	Inventory->OnInventoryStackUpdate.AddDynamic(this, &ThisClass::OnTargetCollectionChanged);
	Inventory->OnCollectionRemovedEvent.AddDynamic(this, &ThisClass::OnTargetCollectionRemoved);

	bEquipmentSystemInitialized = true;
	OnEquipmentSystemInitialized();

	for (const FGIS_ItemInfo& ItemInfo : TargetCollection->GetAllItemInfos())
	{
		FGameplayTag SlotTag = TargetCollection->GetItemSlotName(ItemInfo.Item);
		EquipItemToSlot(ItemInfo.Item, SlotTag);
	}
}

void UGIS_EquipmentSystemComponent::ResetEquipmentSystem()
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}
	RemoveAllEquipments();
	if (IsValid(Inventory))
	{
		Inventory->OnCollectionRemovedEvent.RemoveDynamic(this, &ThisClass::OnTargetCollectionRemoved);
		Inventory->OnInventoryStackUpdate.RemoveDynamic(this, &ThisClass::OnTargetCollectionChanged);
		Inventory = nullptr;
		TargetCollection = nullptr;
		TargetCollectionDefinition = nullptr;
		GroupActiveIdxMap.Empty();
	}
	bEquipmentSystemInitialized = false;
	OnEquipmentSystemInitialized();
}

bool UGIS_EquipmentSystemComponent::IsEquipmentSystemInitialized() const
{
	return bEquipmentSystemInitialized;
}

void UGIS_EquipmentSystemComponent::BindToEquipmentSystemInitialized(FGIS_EquipmentSystem_Initialized_DynamicEvent Delegate)
{
	if (bEquipmentSystemInitialized)
	{
		Delegate.ExecuteIfBound();
	}
	else
	{
		InitializedDelegates.Add(Delegate);
	}
}


UGIS_EquipmentSystemComponent* UGIS_EquipmentSystemComponent::GetEquipmentSystemComponent(const AActor* Actor)
{
	return IsValid(Actor) ? Actor->FindComponentByClass<UGIS_EquipmentSystemComponent>() : nullptr;
}

bool UGIS_EquipmentSystemComponent::FindEquipmentSystemComponent(const AActor* Actor, UGIS_EquipmentSystemComponent*& Component)
{
	Component = (Actor ? Actor->FindComponentByClass<UGIS_EquipmentSystemComponent>() : nullptr);
	return Component != nullptr;
}

bool UGIS_EquipmentSystemComponent::FindTypedEquipmentSystemComponent(AActor* Actor, TSubclassOf<UGIS_EquipmentSystemComponent> DesiredClass, UGIS_EquipmentSystemComponent*& Component)
{
	if (UClass* RealClass = DesiredClass)
	{
		if (FindEquipmentSystemComponent(Actor, Component))
		{
			if (Component->GetClass()->IsChildOf(RealClass))
			{
				return true;
			}
		}
	}
	return false;
}


void UGIS_EquipmentSystemComponent::RemoveAllEquipments()
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}
	for (int32 i = 0; i < Container.Entries.Num(); i++)
	{
		RemoveEquipmentEntry(i);
	}
}

void UGIS_EquipmentSystemComponent::AddEquipmentEntry(const FGIS_EquipmentEntry& NewEntry)
{
	check(NewEntry.IsValid())
	int32 Idx = Container.Entries.AddDefaulted();
	Container.Entries[Idx] = NewEntry;

	AddReplicatedEquipmentObject(NewEntry.Instance);
	OnEquipmentEntryAdded(NewEntry, Idx);
	Container.MarkItemDirty(Container.Entries[Idx]);
}

void UGIS_EquipmentSystemComponent::RemoveEquipmentEntry(int32 Idx)
{
	check(Container.Entries.IsValidIndex(Idx));
	const FGIS_EquipmentEntry& Entry = Container.Entries[Idx];
	RemoveReplicatedEquipmentObject(Entry.Instance);
	OnEquipmentEntryRemoved(Entry, Idx);
	Container.Entries.RemoveAt(Idx);
	Container.MarkArrayDirty();
}

UObject* UGIS_EquipmentSystemComponent::CreateEquipmentInstance_Implementation(AActor* Owner, UGIS_ItemInstance* ItemInstance) const
{
	if (ItemInstance == nullptr)
	{
		GIS_CLOG(Error, "passed in invalid item instance.")
		return nullptr;
	}
	const UGIS_ItemFragment_Equippable* EquippableItem = ItemInstance->FindFragmentByClass<UGIS_ItemFragment_Equippable>();

	if (EquippableItem == nullptr)
	{
		GIS_CLOG(Error, "missing equippable fragment on item(%s)", *ItemInstance->GetDefinition()->GetName())
		return nullptr;
	}

	TSubclassOf<UObject> InstanceType = !EquippableItem->InstanceType.IsNull() ? EquippableItem->InstanceType.LoadSynchronous() : nullptr;
	if (InstanceType == nullptr)
	{
		GIS_CLOG(Error, "missing valid equipment instance type on item(%s)", *ItemInstance->GetDefinition()->GetName()
		)
		return nullptr;
	}
	if (!InstanceType->ImplementsInterface(UGIS_EquipmentInterface::StaticClass()))
	{
		GIS_CLOG(Error, "equipment instance type doesn't implement:%s", *UGIS_EquipmentInterface::StaticClass()->GetName())
		return nullptr;
	}

	UObject* Instance;

	if (EquippableItem->bActorBased)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Owner;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(InstanceType, FTransform::Identity, SpawnParameters);
		if (SpawnedActor == nullptr)
		{
			GIS_CLOG(Error, "failed to create equipment instance of type:%s", *InstanceType->GetName())
			return nullptr;
		}
		Instance = SpawnedActor;
	}
	else
	{
		UGIS_EquipmentInstance* EquipmentInstance = NewObject<UGIS_EquipmentInstance>(Owner, InstanceType); //Using the actor instead of component as the outer due to UE-127172
		if (EquipmentInstance == nullptr)
		{
			GIS_CLOG(Error, "failed to create equipment instance of type:%s", *InstanceType->GetName())
			return nullptr;
		}
		Instance = EquipmentInstance;
	}

	return Instance;
}


void UGIS_EquipmentSystemComponent::OnEquipmentEntryAdded(const FGIS_EquipmentEntry& Entry, int32 Idx)
{
	APawn* OwningPawn = GetPawn<APawn>();

	SlotToIdxMap.Add(Entry.EquippedSlot, Entry.Instance);

	IGIS_EquipmentInterface::Execute_ReceiveOwningPawn(Entry.Instance, OwningPawn);
	IGIS_EquipmentInterface::Execute_ReceiveSourceItem(Entry.Instance, Entry.ItemInstance);
	IGIS_EquipmentInterface::Execute_OnEquipmentBeginPlay(Entry.Instance);
	OnEquipmentStateChangedEvent.Broadcast(Entry.Instance, Entry.EquippedSlot, true);

	if (Entry.bActive)
	{
		IGIS_EquipmentInterface::Execute_OnActiveStateChanged(Entry.Instance, Entry.bActive);
		OnEquipmentActiveStateChangedEvent.Broadcast(Entry.Instance, Entry.EquippedSlot, Entry.bActive);
	}
}

void UGIS_EquipmentSystemComponent::OnEquipmentEntryChanged(const FGIS_EquipmentEntry& Entry, int32 Idx)
{
	IGIS_EquipmentInterface::Execute_OnActiveStateChanged(Entry.Instance, Entry.bActive);
	OnEquipmentActiveStateChangedEvent.Broadcast(Entry.Instance, Entry.EquippedSlot, Entry.bActive);
}

void UGIS_EquipmentSystemComponent::OnEquipmentEntryRemoved(const FGIS_EquipmentEntry& Entry, int32 Idx)
{
	// remove but still active, so notify instance to do deactivate behavior.

	SlotToIdxMap.Remove(Entry.EquippedSlot);

	if (IsValid(Entry.Instance)) // The instance may alreay in pending kill state, so no point to continues execution.
	{
		if (Entry.bActive)
		{
			IGIS_EquipmentInterface::Execute_OnActiveStateChanged(Entry.Instance, false);
		}
		IGIS_EquipmentInterface::Execute_OnEquipmentEndPlay(Entry.Instance);
		IGIS_EquipmentInterface::Execute_ReceiveOwningPawn(Entry.Instance, nullptr);
		IGIS_EquipmentInterface::Execute_ReceiveSourceItem(Entry.Instance, nullptr);
	}

	OnEquipmentStateChangedEvent.Broadcast(Entry.Instance, Entry.EquippedSlot, false);
}

void UGIS_EquipmentSystemComponent::AddReplicatedEquipmentObject(TObjectPtr<UObject> Instance)
{
	if (OwnerHasAuthority() && IsValid(Instance))
	{
		checkf(Instance->GetClass()->ImplementsInterface(UGIS_EquipmentInterface::StaticClass()), TEXT("%s doesn't implement GIS_EquipmentInterface"), *Instance->GetClass()->GetName())
		bool IsReplicationManaged = IGIS_EquipmentInterface::Execute_IsReplicationManaged(Instance);
		if (!IsReplicationManaged)
		{
			return;
		}
		if (IsReadyForReplication() && !IsReplicatedSubObjectRegistered(Instance))
		{
			AddReplicatedSubObject(Instance);
		}
		else
		{
			PendingReplicatedEquipments.AddUnique(Instance);
		}
	}
}

void UGIS_EquipmentSystemComponent::RemoveReplicatedEquipmentObject(TObjectPtr<UObject> Instance)
{
	if (OwnerHasAuthority() && IsValid(Instance))
	{
		bool IsReplicationManaged = IGIS_EquipmentInterface::Execute_IsReplicationManaged(Instance);

		if (IsReplicationManaged && IsReplicatedSubObjectRegistered(Instance))
		{
			RemoveReplicatedSubObject(Instance);
		}
	}
}

#pragma region Equipment Groups

TMap<int32, FGameplayTag> UGIS_EquipmentSystemComponent::GetLayoutOfGroup(FGameplayTag GroupTag) const
{
	TMap<int32, FGameplayTag> GroupLayout;
	if (TargetCollectionDefinition == nullptr)
	{
		return GroupLayout;
	}
	if (TargetCollectionDefinition->SlotGroupMap.Contains(GroupTag))
	{
		GroupLayout = TargetCollectionDefinition->SlotGroupMap[GroupTag].IndexToSlotMap;
	}
	return GroupLayout;
}

TMap<FGameplayTag, int32> UGIS_EquipmentSystemComponent::GetSlottedLayoutOfGroup(FGameplayTag GroupTag) const
{
	TMap<FGameplayTag, int32> GroupLayout;
	if (TargetCollectionDefinition == nullptr)
	{
		return GroupLayout;
	}
	if (TargetCollectionDefinition->SlotGroupMap.Contains(GroupTag))
	{
		GroupLayout = TargetCollectionDefinition->SlotGroupMap[GroupTag].SlotToIndexMap;
	}
	return GroupLayout;
}

TMap<FGameplayTag, UObject*> UGIS_EquipmentSystemComponent::GetSlottedEquipmentsOfGroup(FGameplayTag GroupTag) const
{
	TMap<FGameplayTag, int32> GroupLayout = GetSlottedLayoutOfGroup(GroupTag);

	TMap<FGameplayTag, UObject*> GroupedEquipments;
	GroupedEquipments.Reserve(GroupLayout.Num());

	for (auto& Pair : GroupLayout)
	{
		int32 Idx = Container.IndexOfBySlot(Pair.Key);
		if (Idx != INDEX_NONE)
		{
			GroupedEquipments.Emplace(Pair.Key, Container.Entries[Idx].Instance);
		}
		else
		{
			GroupedEquipments.Emplace(Pair.Key, nullptr);
		}
	}
	return GroupedEquipments;
}

TMap<int32, UObject*> UGIS_EquipmentSystemComponent::GetEquipmentsOfGroup(FGameplayTag GroupTag) const
{
	TMap<int32, FGameplayTag> GroupLayout = GetLayoutOfGroup(GroupTag);

	TMap<int32, UObject*> GroupedEquipments;
	GroupedEquipments.Reserve(GroupLayout.Num());

	for (auto& Pair : GroupLayout)
	{
		int32 Idx = Container.IndexOfBySlot(Pair.Value);
		if (Idx != INDEX_NONE)
		{
			GroupedEquipments.Emplace(Pair.Key, Container.Entries[Idx].Instance);
		}
		else
		{
			GroupedEquipments.Emplace(Pair.Key, nullptr);
		}
	}
	return GroupedEquipments;
}

void UGIS_EquipmentSystemComponent::SetGroupActiveIndex(FGameplayTag GroupTag, int32 NewIndex)
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}
	if (!GroupActiveIdxMap.Contains(GroupTag))
	{
		return;
	}
	int32 PrevActiveIndex = GroupActiveIdxMap[GroupTag];
	if (PrevActiveIndex != NewIndex)
	{
		GroupActiveIdxMap[GroupTag] = NewIndex;
		OnGroupActiveIndexChanged(GroupTag, PrevActiveIndex, NewIndex);
	}
}

void UGIS_EquipmentSystemComponent::ServerSetGroupActiveIndex_Implementation(FGameplayTag GroupTag, int32 NewIndex)
{
	SetGroupActiveIndex(GroupTag, NewIndex);
}


void UGIS_EquipmentSystemComponent::CycleGroupActiveIndex(FGameplayTag GroupTag, bool bDirection)
{
	if (!bEquipmentSystemInitialized || !OwnerHasAuthority())
	{
		GIS_CLOG(Error, "not initialized or has no authority!")
		return;
	}

	if (!GroupActiveIdxMap.Contains(GroupTag))
	{
		GIS_CLOG(Warning, "has no equipment group named:%s.", *GroupTag.ToString())
		return;
	}

	int32 MaxIndex = TargetCollectionDefinition->SlotGroupMap[GroupTag].IndexToSlotMap.Num() - 1;
	int32 PrevIndex = GroupActiveIdxMap[GroupTag];
	if (PrevIndex == INDEX_NONE)
	{
		int32 NewIndex = bDirection ? 0 : MaxIndex;
		GroupActiveIdxMap[GroupTag] = NewIndex;
		OnGroupActiveIndexChanged(GroupTag, PrevIndex, NewIndex);
	}
	else
	{
		int32 NewIndex = bDirection ? PrevIndex + 1 : PrevIndex - 1;
		if (bDirection && NewIndex > MaxIndex)
		{
			NewIndex = 0;
		}
		if (!bDirection && NewIndex < 0)
		{
			NewIndex = MaxIndex;
		}
		if (NewIndex != PrevIndex)
		{
			GroupActiveIdxMap[GroupTag] = NewIndex;
			OnGroupActiveIndexChanged(GroupTag, PrevIndex, NewIndex);
		}
	}
}

void UGIS_EquipmentSystemComponent::ServerCycleGroupActiveIndex_Implementation(FGameplayTag GroupTag, bool bDirection)
{
	CycleGroupActiveIndex(GroupTag, bDirection);
}

void UGIS_EquipmentSystemComponent::OnGroupActiveIndexChanged(const FGameplayTag& GroupTag, int32 PrevIndex, int32 NewIndex)
{
	if (!OwnerHasAuthority())
	{
		return;
	}

	// Deactivate the equipment for prev active index.
	if (PrevIndex != INDEX_NONE && TargetCollectionDefinition->SlotGroupMap[GroupTag].IndexToSlotMap.Contains(PrevIndex))
	{
		FGameplayTag PrevSlot = TargetCollectionDefinition->SlotGroupMap[GroupTag].IndexToSlotMap[PrevIndex];
		int32 EquipmentEntryIdx = Container.IndexOfBySlot(PrevSlot);
		if (EquipmentEntryIdx != INDEX_NONE)
		{
			if (Container.Entries[EquipmentEntryIdx].bActive)
			{
				SetEquipmentActiveState(EquipmentEntryIdx, false);
			}
		}
	}

	// Activate the equipment for new active index.
	if (NewIndex != INDEX_NONE && TargetCollectionDefinition->SlotGroupMap[GroupTag].IndexToSlotMap.Contains(NewIndex))
	{
		FGameplayTag NewSlot = TargetCollectionDefinition->SlotGroupMap[GroupTag].IndexToSlotMap[NewIndex];

		int32 EquipmentEntryIdx = Container.IndexOfBySlot(NewSlot);
		if (EquipmentEntryIdx != INDEX_NONE)
		{
			if (!Container.Entries[EquipmentEntryIdx].bActive)
			{
				SetEquipmentActiveState(EquipmentEntryIdx, true);
			}
		}
	}

	OnEquipmentGroupActiveIndexChangedEvent.Broadcast(GroupTag, PrevIndex, NewIndex);
	ClientNotifyGroupActiveIndexChanged(GroupTag, PrevIndex, NewIndex);
}

void UGIS_EquipmentSystemComponent::ClientNotifyGroupActiveIndexChanged_Implementation(const FGameplayTag& GroupTag, int32 PrevIndex, int32 NewIndex)
{
	OnEquipmentGroupActiveIndexChangedEvent.Broadcast(GroupTag, PrevIndex, NewIndex);
}

#pragma endregion
