// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_SerializationStructLibrary.h"
#include "GIS_ItemFragment.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_SerializationStructLibrary)

bool FGIS_ItemRecord::operator==(const FGIS_ItemRecord& Other) const
{
	return ItemId == Other.ItemId && DefinitionAssetPath == Other.DefinitionAssetPath;
}

bool FGIS_ItemRecord::IsValid() const
{
	return ItemId.IsValid() && !DefinitionAssetPath.IsEmpty();
}

// bool FGIS_ItemFragmentStateRecord::operator==(const FGIS_ItemFragmentStateRecord& Other) const
// {
// 	return FragmentClass == Other.FragmentClass;
// }
//
// bool FGIS_ItemFragmentStateRecord::IsValid() const
// {
// 	return FragmentClass != nullptr && FragmentState.IsValid();
// }

bool FGIS_StackRecord::IsValid() const
{
	return ItemId.IsValid() && Id.IsValid() && CollectionId.IsValid();
}

bool FGIS_CollectionRecord::IsValid() const
{
	return Id.IsValid() && !DefinitionAssetPath.IsEmpty();
}

FGIS_CurrencyRecord::FGIS_CurrencyRecord()
{
	Key = NAME_None;
}
