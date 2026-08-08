// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Items/GIS_ItemInstance.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemDefinition.h"
#include "GIS_ItemFragment.h"
#include "Net/UnrealNetwork.h"
#include "Engine/BlueprintGeneratedClass.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "GIS_LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemInstance)


UGIS_ItemInstance::UGIS_ItemInstance(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), IntegerAttributes(this), FloatAttributes(this), FragmentStates(this)
{
	OwningCollection = nullptr;
}

void UGIS_ItemInstance::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = GetItemTags();
}

void UGIS_ItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	// FastArray don't need push model.
	DOREPLIFETIME(ThisClass, IntegerAttributes);
	DOREPLIFETIME(ThisClass, FloatAttributes);
	DOREPLIFETIME(ThisClass, FragmentStates);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemId, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Definition, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OwningCollection, SharedParams);

	SharedParams.Condition = COND_InitialOrOwner;

	//fix: https://forums.unrealengine.com/t/subobject-replication-for-blueprint-child-class/106205/4
	UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>(this->GetClass());
	if (bpClass != nullptr)
	{
		bpClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
}

FGuid UGIS_ItemInstance::GetItemId() const
{
	return ItemId;
}

void UGIS_ItemInstance::SetItemId(FGuid NewId)
{
	ItemId = NewId;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemId, this);
}

bool UGIS_ItemInstance::IsUnique() const
{
	return Definition->bUnique;
}

FText UGIS_ItemInstance::GetItemName() const
{
	return Definition->DisplayName;
}

const UGIS_ItemDefinition* UGIS_ItemInstance::GetDefinition() const
{
	return Definition;
}

void UGIS_ItemInstance::SetDefinition(const UGIS_ItemDefinition* NewDefinition)
{
	Definition = NewDefinition;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Definition, this);
}

FGameplayTagContainer UGIS_ItemInstance::GetItemTags() const
{
	return Definition->ItemTags;
}

const UGIS_ItemFragment* UGIS_ItemInstance::GetFragment(TSubclassOf<UGIS_ItemFragment> FragmentClass) const
{
	if (Definition != nullptr && FragmentClass != nullptr)
	{
		return Definition->GetFragment(FragmentClass);
	}
	return nullptr;
}

const UGIS_ItemFragment* UGIS_ItemInstance::FindFragment(TSubclassOf<UGIS_ItemFragment> FragmentClass, bool& bValid) const
{
	bValid = false;
	if (const UGIS_ItemFragment* Fragment = GetFragment(FragmentClass))
	{
		bValid = true;
		return Fragment;
	}
	return nullptr;
}

bool UGIS_ItemInstance::HasAnyAttribute(FGameplayTag AttributeTag) const
{
	return HasFloatAttribute(AttributeTag) || HasIntegerAttribute(AttributeTag);
}

bool UGIS_ItemInstance::HasFloatAttribute(FGameplayTag AttributeTag) const
{
	if (FloatAttributes.ContainsTag(AttributeTag))
	{
		return true;
	}
	return false;
}

FText UGIS_ItemInstance::GetItemDescription() const
{
	return Definition->Description;
}

float UGIS_ItemInstance::GetFloatAttribute(FGameplayTag AttributeTag) const
{
	if (FloatAttributes.ContainsTag(AttributeTag))
	{
		return FloatAttributes.GetValue(AttributeTag);
	}
	return 0;
}

int32 UGIS_ItemInstance::GetIntegerAttribute(FGameplayTag AttributeTag) const
{
	if (IntegerAttributes.ContainsTag(AttributeTag))
	{
		return IntegerAttributes.GetValue(AttributeTag);
	}
	return 0;
}

void UGIS_ItemInstance::SetFloatAttribute(FGameplayTag AttributeTag, float NewValue)
{
	FloatAttributes.SetItem(AttributeTag, NewValue);
}

void UGIS_ItemInstance::AddFloatAttribute(FGameplayTag AttributeTag, float Value)
{
	FloatAttributes.AddItem(AttributeTag, Value);
}

void UGIS_ItemInstance::RemoveFloatAttribute(FGameplayTag AttributeTag, float Value)
{
	FloatAttributes.RemoveItem(AttributeTag, Value);
}

bool UGIS_ItemInstance::HasIntegerAttribute(FGameplayTag AttributeTag) const
{
	if (IntegerAttributes.ContainsTag(AttributeTag))
	{
		return true;
	}
	return false;
}

void UGIS_ItemInstance::SetIntegerAttribute(FGameplayTag AttributeTag, int32 NewValue)
{
	IntegerAttributes.SetItem(AttributeTag, NewValue);
}

void UGIS_ItemInstance::AddIntegerAttribute(FGameplayTag AttributeTag, int32 NewValue)
{
	IntegerAttributes.AddItem(AttributeTag, NewValue);
}

void UGIS_ItemInstance::RemoveIntegerAttribute(FGameplayTag AttributeTag, int32 Value)
{
	IntegerAttributes.RemoveItem(AttributeTag, Value);
}

UGIS_ItemCollection* UGIS_ItemInstance::GetOwningCollection() const
{
	return OwningCollection;
}

UGIS_InventorySystemComponent* UGIS_ItemInstance::GetOwningInventory() const
{
	return IsValid(OwningCollection) ? OwningCollection->GetOwningInventory() : nullptr;
}

bool UGIS_ItemInstance::FindFragmentStateByClass(TSubclassOf<UGIS_ItemFragment> FragmentClass, FInstancedStruct& OutState) const
{
	return FragmentStates.GetDataByTarget(GetFragment(FragmentClass), OutState);
}

void UGIS_ItemInstance::SetFragmentStateByClass(TSubclassOf<UGIS_ItemFragment> FragmentClass, const FInstancedStruct& NewState)
{
	if (const UGIS_ItemFragment* Fragment = Definition->GetFragment(FragmentClass))
	{
		FragmentStates.SetDataForTarget(Fragment, NewState);
	}
}

void UGIS_ItemInstance::AssignCollection(UGIS_ItemCollection* NewItemCollection)
{
	if (NewItemCollection == nullptr || OwningCollection == NewItemCollection)
	{
		return;
	}

	if (OwningCollection != nullptr && OwningCollection->GetOwningInventory() != nullptr && OwningCollection != NewItemCollection)
	{
		GIS_CLOG(Error, "is unable to be added to a new item collection:%s when it is already a member of an existing item collection.", *OwningCollection->GetDebugString());
		return;
	}
	OwningCollection = NewItemCollection;
	GIS_CLOG(Verbose, "item(%s) assigned new collection(%s)", *GetNameSafe(this), *OwningCollection->GetDebugString());
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwningCollection, this);
}

void UGIS_ItemInstance::UnassignCollection(UGIS_ItemCollection* ItemCollection)
{
	if (OwningCollection != nullptr && ItemCollection != nullptr && OwningCollection != ItemCollection)
	{
		GIS_CLOG(Warning, "belong to %s, but trying to remove from %s.", *OwningCollection->GetDebugString(), *ItemCollection->GetDebugString());
		return;
	}
	GIS_CLOG(Verbose, "item(%s) removed from collection(%s)", *GetNameSafe(this), *OwningCollection->GetDebugString());
	OwningCollection = nullptr;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwningCollection, this);
}

// void UGIS_ItemInstance::ResetCollection()
// {
// 	OwningCollection = nullptr;
// 	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwningCollection, this);
// }

bool UGIS_ItemInstance::IsItemValid() const
{
	return ItemId.IsValid() && Definition != nullptr;
}

bool UGIS_ItemInstance::StackableEquivalentTo(const UGIS_ItemInstance* OtherItem) const
{
	return AreStackableEquivalent(this, OtherItem);
}

bool UGIS_ItemInstance::SimilarTo(const UGIS_ItemInstance* OtherItem) const
{
	return AreSimilar(this, OtherItem);
}

bool UGIS_ItemInstance::AreStackableEquivalent(const UGIS_ItemInstance* Lhs, const UGIS_ItemInstance* Rhs)
{
	if (Lhs == nullptr || Rhs == nullptr)
	{
		return false;
	}
	// 有必要比较同一个指针吗?
	if (Lhs == Rhs)
	{
		return true;
	}
	if (Lhs->GetClass() != Rhs->GetClass())
	{
		return false;
	}
	if (Lhs->GetDefinition() != nullptr && Rhs->GetDefinition() != nullptr)
	{
		if (Lhs->GetDefinition() != Rhs->GetDefinition())
		{
			return false;
		}
	}
	if (Lhs->IsUnique())
	{
		return false;
	}
	return true;
}

bool UGIS_ItemInstance::AreSimilar(const UGIS_ItemInstance* Lhs, const UGIS_ItemInstance* Rhs)
{
	if (Lhs == nullptr || Rhs == nullptr)
	{
		return false;
	}
	if (Lhs == Rhs)
	{
		return true;
	}
	if (Lhs->GetClass() != Rhs->GetClass())
	{
		return false;
	}
	if (Lhs->GetDefinition() != Rhs->GetDefinition())
	{
		return false;
	}
	if (Lhs->IsUnique())
	{
		return false;
	}
	return true;
}

void UGIS_ItemInstance::OnItemDuplicated(const UGIS_ItemInstance* SrcItem)
{
}

const FGIS_MixinContainer& UGIS_ItemInstance::GetFragmentStates() const
{
	return FragmentStates;
}

void UGIS_ItemInstance::OnMixinDataAdded(const TObjectPtr<const UObject>& Target, const FInstancedStruct& Data)
{
	const UGIS_ItemFragment* Fragment = CastChecked<UGIS_ItemFragment>(Target);
	OnFragmentStateAdded(Fragment, Data);
	OnFragmentStateAddedEvent.Broadcast(Fragment, Data);
}

void UGIS_ItemInstance::OnMixinDataUpdated(const TObjectPtr<const UObject>& Target, const FInstancedStruct& Data)
{
	const UGIS_ItemFragment* Fragment = CastChecked<UGIS_ItemFragment>(Target);
	OnFragmentStateUpdated(Fragment, Data);
	OnFragmentStateUpdatedEvent.Broadcast(Fragment, Data);
}

void UGIS_ItemInstance::OnMixinDataRemoved(const TObjectPtr<const UObject>& Target, const FInstancedStruct& Data)
{
	const UGIS_ItemFragment* Fragment = CastChecked<UGIS_ItemFragment>(Target);
	OnFragmentStateRemoved(Fragment, Data);
	OnFragmentStateRemovedEvent.Broadcast(Fragment, Data);
}

void UGIS_ItemInstance::OnFragmentStateAdded(const UGIS_ItemFragment* Fragment, const FInstancedStruct& Data)
{
}

void UGIS_ItemInstance::OnFragmentStateUpdated(const UGIS_ItemFragment* Fragment, const FInstancedStruct& Data)
{
}

void UGIS_ItemInstance::OnFragmentStateRemoved(const UGIS_ItemFragment* Fragment, const FInstancedStruct& Data)
{
}

void UGIS_ItemInstance::OnTagFloatUpdate(const FGameplayTag& Tag, float OldValue, float NewValue)
{
	OnFloatAttributeChanged(Tag, OldValue, NewValue);
	OnFloatAttributeChangedEvent.Broadcast(Tag, OldValue, NewValue);
}

void UGIS_ItemInstance::OnTagIntegerUpdate(const FGameplayTag& Tag, int32 OldValue, int32 NewValue)
{
	OnIntegerAttributeChanged(Tag, OldValue, NewValue);
	OnIntegerAttributeChangedEvent.Broadcast(Tag, OldValue, NewValue);
}

void UGIS_ItemInstance::OnFloatAttributeChanged_Implementation(const FGameplayTag& Tag, float OldValue, float NewValue)
{
}

void UGIS_ItemInstance::OnIntegerAttributeChanged_Implementation(const FGameplayTag& Tag, int32 OldValue, int32 NewValue)
{
}

#if UE_WITH_IRIS
void UGIS_ItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;
	UObject::RegisterReplicationFragments(Context, RegistrationFlags);
	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif
