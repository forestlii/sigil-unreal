// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_InventoryFactory.h"
#include "UObject/Object.h"
#include "GameFramework/Actor.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "GIS_CollectionContainer.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemDefinition.h"
#include "GIS_ItemFragment.h"
#include "GIS_LogChannels.h"
#include "Items/GIS_ItemInstance.h"
#include "Misc/DataValidation.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_InventoryFactory)

UGIS_ItemInstance* UGIS_InventoryFactory::DuplicateItem_Implementation(AActor* Owner, UGIS_ItemInstance* SrcItem, bool bGenerateNewId)
{
	if (!IsValid(SrcItem) || SrcItem->GetDefinition() == nullptr)
	{
		GIS_LOG(Error, "Missing src item or src item doesn't have valid definition.");
		return nullptr;
	}
	UGIS_ItemInstance* NewItem = DuplicateObject(SrcItem, Owner);
	if (bGenerateNewId)
	{
		NewItem->SetItemId(FGuid::NewGuid());
	}
	NewItem->OnItemDuplicated(SrcItem);
	return NewItem;
}

UGIS_ItemCollection* UGIS_InventoryFactory::CreateCollection_Implementation(AActor* Owner, const UGIS_ItemCollectionDefinition* Definition)
{
	if (!IsValid(Owner))
	{
		GIS_LOG(Error, "Missing owner.");
		return nullptr;
	}
	if (!IsValid(Definition))
	{
		GIS_LOG(Error, "Cannot create collection with null collection Definition.");
		return nullptr;
	}
	TSubclassOf<UGIS_ItemCollection> CollectionClass = Definition->GetCollectionInstanceClass();
	if (CollectionClass == nullptr)
	{
		GIS_LOG(Error, "definition(%s) doesn't specify valid item collection class.", *Definition->GetName())
		return nullptr;
	}
	UGIS_ItemCollection* NewCollection = NewObject<UGIS_ItemCollection>(Owner, CollectionClass);
	if (NewCollection == nullptr)
	{
		GIS_LOG(Error, "failed to create instance of %s", *GetNameSafe(Definition))
		return nullptr;
	}
	return NewCollection;
}

UGIS_InventoryFactory::UGIS_InventoryFactory()
{
	DefaultItemInstanceClass = UGIS_ItemInstance::StaticClass();
}

UGIS_ItemInstance* UGIS_InventoryFactory::CreateItem_Implementation(AActor* Owner, const UGIS_ItemDefinition* ItemDefinition)
{
	if (!IsValid(Owner))
	{
		GIS_LOG(Error, "Missing owner.");
		return nullptr;
	}

	if (ItemDefinition == nullptr)
	{
		GIS_LOG(Error, "Cannot create Item with null Item Definition.");
		return nullptr;
	}

	TSubclassOf<UGIS_ItemInstance> ItemInstanceClass = DefaultItemInstanceClass.LoadSynchronous();

	if (ItemInstanceClass == nullptr)
	{
		GIS_LOG(Error, "ItemDefinition: %s has invalid InstanceType.", *ItemDefinition->GetName());
		return nullptr;
	}

	UGIS_ItemInstance* Item = NewObject<UGIS_ItemInstance>(Owner, ItemInstanceClass);
	if (Item == nullptr)
	{
		GIS_LOG(Error, "ItemInstanceClass: %s create failed.", *ItemInstanceClass->GetName());
		return nullptr;
	}

	Item->SetItemId(FGuid::NewGuid());
	Item->SetDefinition(ItemDefinition);

	for (const UGIS_ItemFragment* Fragment : ItemDefinition->Fragments)
	{
		if (Fragment == nullptr)
		{
			continue;
		}
		if (Fragment->GetCompatibleMixinDataType() != nullptr)
		{
			FInstancedStruct FragmentState;
			if (Fragment->MakeDefaultMixinData(FragmentState))
			{
				if (FragmentState.IsValid() && FragmentState.GetScriptStruct() == Fragment->GetCompatibleMixinDataType())
				{
					Item->SetFragmentStateByClass(Fragment->GetClass(), FragmentState);
				}
			}
		}
		Fragment->OnInstanceCreated(Item);
	}
	return Item;
}

UGIS_ItemInstance* UGIS_InventoryFactory::DeserializeItem_Implementation(AActor* Owner, const FGIS_ItemRecord& Record)
{
	if (!IsValid(Owner))
	{
		GIS_LOG(Error, "Missing owner.");
		return nullptr;
	}

	const FSoftObjectPath ItemDefinitionAssetPath = FSoftObjectPath(Record.DefinitionAssetPath);
	const TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinitionReference = TSoftObjectPtr<UGIS_ItemDefinition>(ItemDefinitionAssetPath);

	UGIS_ItemDefinition* ItemDefinition = !ItemDefinitionReference.IsNull() ? ItemDefinitionReference.LoadSynchronous() : nullptr;
	if (!IsValid(ItemDefinition))
	{
		GIS_LOG(Warning, "invalid item definition on path:%s", *ItemDefinitionAssetPath.ToString());
		return nullptr;
	}

	UGIS_ItemInstance* ItemInstance = CreateItem(Owner, ItemDefinition);
	if (!IsValid(ItemInstance))
	{
		GIS_LOG(Warning, "failed to create item instance from definition:%s", *GetNameSafe(ItemDefinition));
		return nullptr;
	}

	ItemInstance->SetItemId(Record.ItemId);
	ItemInstance->SetDefinition(ItemDefinition);

	TArray<FGIS_Mixin> ConvertedMixins = FGIS_MixinContainer::ConvertRecordsToMixins(Record.FragmentStateRecords);
	ConvertedMixins = ConvertedMixins.FilterByPredicate([ItemDefinition](const FGIS_Mixin& Mixin)
	{
		return ItemDefinition->Fragments.Contains(Mixin.Target);
	});

	for (const FGIS_Mixin& ConvertedMixin : ConvertedMixins)
	{
		ItemInstance->SetFragmentStateByClass(ConvertedMixin.Target->GetClass(), ConvertedMixin.Data);
	}

	FMemoryReader Reader(Record.ByteData);
	FObjectAndNameAsStringProxyArchive Ar2(Reader, true);
	Ar2.ArIsSaveGame = true;
	ItemInstance->Serialize(Ar2);
	return ItemInstance;
}

bool UGIS_InventoryFactory::SerializeItem_Implementation(UGIS_ItemInstance* Item, FGIS_ItemRecord& Record)
{
	if (!IsValid(Item))
	{
		GIS_LOG(Error, "Missing item.");
		return false;
	}

	Record.ItemId = Item->GetItemId();
	const FSoftObjectPath AssetPath = FSoftObjectPath(Item->GetDefinition());
	Record.DefinitionAssetPath = AssetPath.ToString();
	Record.FragmentStateRecords = Item->GetFragmentStates().GetSerializableMixinRecords();


	FMemoryWriter Writer(Record.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(Writer, true);
	Ar.ArIsSaveGame = true;
	Item->Serialize(Ar);
	return true;
}

bool UGIS_InventoryFactory::SerializeCollection_Implementation(UGIS_ItemCollection* Collection, FGIS_CollectionRecord& Record)
{
	if (!IsValid(Collection))
	{
		GIS_LOG(Error, "Missing collection.");
		return false;
	}

	Record.Tag = Collection->GetCollectionTag();
	Record.Id = Collection->GetCollectionId();
	const FSoftObjectPath AssetPath = FSoftObjectPath(Collection->GetDefinition());
	Record.DefinitionAssetPath = AssetPath.ToString();
	const TArray<FGIS_ItemStack>& ValidStacks = Collection->GetAllItemStacks().FilterByPredicate([](const FGIS_ItemStack& ItemStack)
	{
		return ItemStack.IsValidStack();
	});

	for (const FGIS_ItemStack& Stack : ValidStacks)
	{
		FGIS_StackRecord StackRecord;
		StackRecord.Id = Stack.Id;
		StackRecord.CollectionId = Stack.Collection->GetCollectionId();
		StackRecord.ItemId = Stack.Item->GetItemId();
		StackRecord.Amount = Stack.Amount;
		Record.StackRecords.Add(StackRecord);
	}

	return Record.IsValid();
}

void UGIS_InventoryFactory::DeserializeCollection_Implementation(UGIS_InventorySystemComponent* InventorySystem, const FGIS_CollectionRecord& Record, TMap<FGuid, UGIS_ItemInstance*>& ItemsMap)
{
	if (!IsValid(InventorySystem))
	{
		GIS_LOG(Error, "Missing inventory system.");
		return;
	}
	const FSoftObjectPath DefinitionAssetPath = FSoftObjectPath(Record.DefinitionAssetPath);
	const TSoftObjectPtr<UGIS_ItemCollectionDefinition> DefinitionReference = TSoftObjectPtr<UGIS_ItemCollectionDefinition>(DefinitionAssetPath);
	UGIS_ItemCollectionDefinition* Definition = DefinitionReference.LoadSynchronous();

	if (Definition == nullptr)
	{
		GIS_LOG(Error, "failed to load definition from path:%s", *DefinitionAssetPath.ToString());
		return;
	}

	UGIS_ItemCollection* NewCollection = CreateCollection(InventorySystem->GetOwner(), Definition);
	if (NewCollection == nullptr)
	{
		GIS_LOG(Error, "failed to create collection from definition:%s", *GetNameSafe(Definition));
		return;
	}

	FGIS_CollectionEntry NewEntry;
	NewEntry.Id = Record.Id;
	NewEntry.Instance = NewCollection;
	NewEntry.Definition = Definition;
	InventorySystem->AddCollectionEntry(NewEntry);

	for (const FGIS_StackRecord& StackRecord : Record.StackRecords)
	{
		FGIS_ItemInfo Info;
		Info.Item = ItemsMap[StackRecord.ItemId];
		Info.Amount = StackRecord.Amount;
		Info.ItemCollection = NewCollection;
		InventorySystem->AddItem(Info);
	}
}

bool UGIS_InventoryFactory::SerializeInventory_Implementation(UGIS_InventorySystemComponent* InventorySystem, FGIS_InventoryRecord& Record)
{
	if (!IsValid(InventorySystem))
	{
		GIS_LOG(Error, "Missing inventory system.");
		return false;
	}

	TArray<FGIS_CollectionRecord> CollectionRecords;
	TArray<FGIS_ItemRecord> ItemRecords;

	TArray<UGIS_ItemCollection*> Collections = InventorySystem->GetItemCollections();
	TArray<UGIS_ItemInstance*> Items;

	// build collection records.
	for (UGIS_ItemCollection* Collection : Collections)
	{
		if (Collection->IsInitialized())
		{
			FGIS_CollectionRecord CollectionRecord;
			if (SerializeCollection(Collection, CollectionRecord))
			{
				CollectionRecords.Add(CollectionRecord);
			}
			Items.Append(Collection->GetAllItems());
		}
	}

	ItemRecords.Reserve(Items.Num());
	for (UGIS_ItemInstance* Item : Items)
	{
		FGIS_ItemRecord ItemRecord;
		if (SerializeItem(Item, ItemRecord))
		{
			ItemRecords.Add(ItemRecord);
		}
	}

	Record.ItemRecords = ItemRecords;
	Record.CollectionRecords = CollectionRecords;

	return true;
}

void UGIS_InventoryFactory::DeserializeInventory_Implementation(UGIS_InventorySystemComponent* InventorySystem, const FGIS_InventoryRecord& InRecord)
{
	if (!IsValid(InventorySystem))
	{
		GIS_LOG(Error, "Missing inventory system.");
		return;
	}

	TMap<FGuid, UGIS_ItemInstance*> ItemsMap;
	for (const FGIS_ItemRecord& ItemRecord : InRecord.ItemRecords)
	{
		if (UGIS_ItemInstance* Instance = DeserializeItem(InventorySystem->GetOwner(), ItemRecord))
		{
			ItemsMap.Emplace(Instance->GetItemId(), Instance);
		}
	}

	for (const FGIS_CollectionRecord& CollectionRecord : InRecord.CollectionRecords)
	{
		DeserializeCollection(InventorySystem, CollectionRecord, ItemsMap);
	}

	if (!InventorySystem->IsDefaultCollectionCreated())
	{
		GIS_OWNED_CLOG(InventorySystem, Warning,
		               "The default collection definitions is not match with collections restored from inventory record. That may be a problem as you changed the layout of inventory.")
	}
}

// TArray<FGIS_ItemFragmentStateRecord> UGIS_InventoryFactory::FilterSerializableFragmentStates(const UGIS_ItemInstance* ItemInstance)
// {
// 	TArray<FGIS_Mixin> Mixins = ItemInstance->GetFragmentStates().GetSerializableMixins();
// 	TArray<FGIS_ItemFragmentStateRecord> Records;
// 	for (const FGIS_Mixin& Mixin : Mixins)
// 	{
// 		if (Mixin.Target->IsA<UGIS_ItemFragment>())
// 		{
// 			FGIS_ItemFragmentStateRecord Record;
// 			Record.FragmentClass = Mixin.Target->GetClass();
// 			Record.FragmentState = Mixin.Data;
// 			Records.Add(Record);
// 		}
// 	}
// 	return Records;
// }

// TArray<FGIS_ItemFragmentStateRecord> UGIS_InventoryFactory::FilterCompatibleFragmentStateRecords(const UGIS_ItemDefinition* ItemDefinition, const FGIS_ItemRecord& Record)
// {
// 	TArray<FGIS_Mixin> ConvertedMixins = FGIS_MixinContainer::ConvertRecordsToMixins(Record.FragmentStateRecords);
//
// 	TArray<FGIS_ItemFragmentStateRecord> CompatibleRecords;
// 	for (const FGIS_ItemFragmentStateRecord& StateRecord : Record.FragmentStateRecords)
// 	{
// 		if (StateRecord.FragmentClass == nullptr || !StateRecord.FragmentState.IsValid())
// 		{
// 			GIS_LOG(Warning, "Skip restoring invalid fragment state for item:%s", *ItemDefinition->GetName());
// 			continue;
// 		}
// 		const UGIS_ItemFragment* Fragment = ItemDefinition->GetFragment(StateRecord.FragmentClass);
//
// 		if (Fragment == nullptr)
// 		{
// 			GIS_LOG(Warning, "Skip restoring fragment's state, as fragment(%s) existed in record no longer exists on item(%s).",
// 			        *GetNameSafe(StateRecord.FragmentClass), *ItemDefinition->GetName());
// 			continue;
// 		}
//
// 		if (!Fragment->IsMixinDataSerializable())
// 		{
// 			GIS_LOG(Warning, "Skip restoring fragment's state, as fragment(%s) existed in record no longer considered serializable on item(%s).",
// 			        *GetNameSafe(StateRecord.FragmentClass), *ItemDefinition->GetName());
// 			continue;
// 		}
//
// 		if (Fragment->GetCompatibleMixinDataType() != StateRecord.FragmentState.GetScriptStruct())
// 		{
// 			GIS_LOG(Warning,
// 			        "Skip restoring fragment's state, as fragment(%s)'s state type(%s) in record no longer compatible with the new type(%s) on item(%s).",
// 			        *GetNameSafe(StateRecord.FragmentClass), *GetNameSafe(StateRecord.FragmentState.GetScriptStruct()), *GetNameSafe(Fragment->GetCompatibleMixinDataType()),
// 			        *ItemDefinition->GetName());
// 		}
// 		CompatibleRecords.Add(StateRecord);
// 	}
// 	return CompatibleRecords;
// }

#if WITH_EDITOR
EDataValidationResult UGIS_InventoryFactory::IsDataValid(class FDataValidationContext& Context) const
{
	if (DefaultItemInstanceClass.IsNull())
	{
		Context.AddError(FText::FromString(TEXT("Missing Default Item Instance Class")));
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
