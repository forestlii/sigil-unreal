// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_ItemFragment_Equippable.h"

#include "GIS_EquipmentInstance.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemFragment_Equippable)


UGIS_ItemFragment_Equippable::UGIS_ItemFragment_Equippable()
{
	InstanceType = UGIS_EquipmentInstance::StaticClass();
}

#if WITH_EDITOR
void UGIS_ItemFragment_Equippable::PreSave(FObjectPreSaveContext SaveContext)
{
	if (InstanceType.IsNull())
	{
		bActorBased = false;
	}
	else
	{
		UClass* InstanceClass = InstanceType.LoadSynchronous();
		bActorBased = InstanceClass->IsChildOf(AActor::StaticClass());
	}
	Super::PreSave(SaveContext);
}
#endif
