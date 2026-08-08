// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemFragment_DynamicAttributes.h"

#include "Items/GIS_ItemInstance.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemFragment_DynamicAttributes)

void UGIS_ItemFragment_DynamicAttributes::OnInstanceCreated(UGIS_ItemInstance* Instance) const
{
	for (int32 i = 0; i < InitialFloatAttributes.Num(); i++)
	{
		if (InitialFloatAttributes[i].Tag.IsValid())
		{
			Instance->SetFloatAttribute(InitialFloatAttributes[i].Tag, InitialFloatAttributes[i].Value);
		}
	}
	for (int32 i = 0; i < InitialIntegerAttributes.Num(); i++)
	{
		if (InitialIntegerAttributes[i].Tag.IsValid())
		{
			Instance->SetIntegerAttribute(InitialIntegerAttributes[i].Tag, InitialIntegerAttributes[i].Value);
		}
	}
}

float UGIS_ItemFragment_DynamicAttributes::GetFloatAttributeDefault(FGameplayTag AttributeTag) const
{
	return FloatAttributeMap.Contains(AttributeTag) ? FloatAttributeMap[AttributeTag] : 0;
}

int32 UGIS_ItemFragment_DynamicAttributes::GetIntegerAttributeDefault(FGameplayTag AttributeTag) const
{
	return IntegerAttributeMap.Contains(AttributeTag) ? IntegerAttributeMap[AttributeTag] : 0;
}

#if WITH_EDITOR
void UGIS_ItemFragment_DynamicAttributes::PreSave(FObjectPreSaveContext SaveContext)
{
	FloatAttributeMap.Empty();
	for (const FGIS_GameplayTagFloat& Attribute : InitialFloatAttributes)
	{
		FloatAttributeMap.Add(Attribute.Tag, Attribute.Value);
	}

	IntegerAttributeMap.Empty();
	for (const FGIS_GameplayTagInteger& Attribute : InitialIntegerAttributes)
	{
		IntegerAttributeMap.Add(Attribute.Tag, Attribute.Value);
	}

	Super::PreSave(SaveContext);
}
#endif
