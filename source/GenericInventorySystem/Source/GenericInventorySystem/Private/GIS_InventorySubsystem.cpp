// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_InventorySubsystem.h"
#include "Engine/World.h"
#include "GIS_InventorySystemSettings.h"
#include "GIS_InventoryFactory.h"
#include "GIS_LogChannels.h"
#include "Items/GIS_ItemDefinition.h"
#include "Items/GIS_ItemInstance.h"
#include "Items/GIS_ItemInterface.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_InventorySubsystem)

UGIS_InventorySubsystem* UGIS_InventorySubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGIS_InventorySubsystem>();
	}
	return nullptr;
}

void UGIS_InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	InitializeFactory();
}

void UGIS_InventorySubsystem::Deinitialize()
{
	Super::Deinitialize();
	Factory = nullptr;
}

UGIS_ItemInstance* UGIS_InventorySubsystem::CreateItem(AActor* Owner, TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition)
{
	if (Factory && !ItemDefinition.IsNull())
	{
		UGIS_ItemDefinition* LoadedDefinition = ItemDefinition.LoadSynchronous();
		check(LoadedDefinition)

		if (LoadedDefinition == nullptr)
		{
			GIS_LOG(Error, "Cannot create Item with null Item Definition.");
			return nullptr;
		}

		return Factory->CreateItem(Owner, LoadedDefinition);
	}
	return nullptr;
}

UGIS_ItemInstance* UGIS_InventorySubsystem::CreateItem(AActor* Owner, const UGIS_ItemDefinition* ItemDefinition)
{
	if (Factory && ItemDefinition != nullptr)
	{
		return Factory->CreateItem(Owner, ItemDefinition);
	}
	return nullptr;
}

UGIS_ItemInstance* UGIS_InventorySubsystem::DuplicateItem(AActor* Owner, UGIS_ItemInstance* FromItem, bool bGenerateNewId)
{
	if (Factory)
	{
		return Factory->DuplicateItem(Owner, FromItem, bGenerateNewId);
	}
	return nullptr;
}

bool UGIS_InventorySubsystem::SerializeItem(UGIS_ItemInstance* Item, FGIS_ItemRecord& Record)
{
	if (Factory)
	{
		return Factory->SerializeItem(Item, Record);
	}
	return false;
}

UGIS_ItemInstance* UGIS_InventorySubsystem::DeserializeItem(AActor* Owner, const FGIS_ItemRecord& Record)
{
	if (Factory)
	{
		return Factory->DeserializeItem(Owner, Record);
	}
	return nullptr;
}

bool UGIS_InventorySubsystem::SerializeCollection(UGIS_ItemCollection* ItemCollection, FGIS_CollectionRecord& Record)
{
	if (Factory)
	{
		return Factory->SerializeCollection(ItemCollection, Record);
	}
	return false;
}

void UGIS_InventorySubsystem::DeserializeCollection(UGIS_InventorySystemComponent* InventorySystem, const FGIS_CollectionRecord& Record, TMap<FGuid, UGIS_ItemInstance*>& ItemsMap)
{
	if (Factory)
	{
		return Factory->DeserializeCollection(InventorySystem, Record, ItemsMap);
	}
}

bool UGIS_InventorySubsystem::SerializeInventory(UGIS_InventorySystemComponent* InventorySystem, FGIS_InventoryRecord& Record)
{
	if (Factory)
	{
		return Factory->SerializeInventory(InventorySystem, Record);
	}
	return false;
}

void UGIS_InventorySubsystem::DeserializeInventory(UGIS_InventorySystemComponent* InventorySystem, const FGIS_InventoryRecord& Record)
{
	if (Factory)
	{
		return Factory->DeserializeInventory(InventorySystem, Record);
	}
}


void UGIS_InventorySubsystem::InitializeFactory()
{
	if (UGIS_InventorySystemSettings::Get() == nullptr || UGIS_InventorySystemSettings::Get()->InventoryFactoryClass.IsNull())
	{
		GIS_LOG(Error, "Missing ItemFactoryClass in inventory system settings.");
		return;
	}
	const UClass* FactoryClass = UGIS_InventorySystemSettings::Get()->InventoryFactoryClass.LoadSynchronous();

	if (FactoryClass == nullptr)
	{
		GIS_LOG(Error, "invalid ItemFactoryClass found inventory system settings.");
		return;
	}

	UGIS_InventoryFactory* TempFactory = NewObject<UGIS_InventoryFactory>(this, FactoryClass);
	if (TempFactory == nullptr)
	{
		GIS_LOG(Error, "Failed to create item factory instance.Class:%s", *FactoryClass->GetName());
		return;
	}
	Factory = TempFactory;
}
