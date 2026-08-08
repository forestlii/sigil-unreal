// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_InventorySystemSettings.h"
#include "GIS_InventoryFactory.h"
#include "Items/GIS_ItemDefinitionSchema.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_InventorySystemSettings)

UGIS_InventorySystemSettings::UGIS_InventorySystemSettings()
{
	InventoryFactoryClass = UGIS_InventoryFactory::StaticClass();
}

FName UGIS_InventorySystemSettings::GetCategoryName() const
{
	return TEXT("Game");
}

const UGIS_InventorySystemSettings* UGIS_InventorySystemSettings::Get()
{
	return GetDefault<UGIS_InventorySystemSettings>();
}

const UGIS_ItemDefinitionSchema* UGIS_InventorySystemSettings::GetItemDefinitionSchemaForAsset(const FString& AssetPath) const
{
	// Check path-specific schemas first
	for (const FGIS_ItemDefinitionSchemaEntry& Entry : ItemDefinitionSchemaMap)
	{
		if (!Entry.PathPrefix.IsEmpty() && AssetPath.StartsWith(Entry.PathPrefix))
		{
			if (Entry.Schema.IsValid())
			{
				if (UGIS_ItemDefinitionSchema* Schema = Cast<UGIS_ItemDefinitionSchema>(Entry.Schema.TryLoad()))
				{
					UE_LOG(LogTemp, Log, TEXT("Using path-specific schema %s for asset %s"), *Entry.Schema.ToString(), *AssetPath);
					return Schema;
				}
			}
		}
	}

	// Fall back to default schema
	if (DefaultItemDefinitionSchema.IsValid())
	{
		if (UGIS_ItemDefinitionSchema* Schema = Cast<UGIS_ItemDefinitionSchema>(DefaultItemDefinitionSchema.TryLoad()))
		{
			UE_LOG(LogTemp, Log, TEXT("Using default schema %s for asset %s"), *DefaultItemDefinitionSchema.ToString(), *AssetPath);
			return Schema;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No valid schema found for asset %s"), *AssetPath);
	return nullptr;
}
