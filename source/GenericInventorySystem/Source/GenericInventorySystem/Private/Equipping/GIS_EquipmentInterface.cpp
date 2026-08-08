// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GIS_EquipmentInterface.h"
#include "GameFramework/Pawn.h"
#include "GIS_EquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_EquipmentInterface)

void IGIS_EquipmentInterface::ReceiveOwningPawn_Implementation(APawn* NewPawn)
{
}

APawn* IGIS_EquipmentInterface::GetOwningPawn_Implementation() const
{
	APawn* ReturnPawn = Cast<APawn>(_getUObject()->GetOuter());
	return ReturnPawn;
}

void IGIS_EquipmentInterface::ReceiveSourceItem_Implementation(UGIS_ItemInstance* NewItem)
{
}

UGIS_ItemInstance* IGIS_EquipmentInterface::GetSourceItem_Implementation() const
{
	return nullptr;
}

void IGIS_EquipmentInterface::OnEquipped_Implementation()
{
}

void IGIS_EquipmentInterface::OnActiveStateChanged_Implementation(bool bNewActiveState)
{
}

bool IGIS_EquipmentInterface::IsEquipmentActive_Implementation() const
{
	return false;
}

void IGIS_EquipmentInterface::OnEquipmentBeginPlay_Implementation()
{
}

void IGIS_EquipmentInterface::OnEquipmentEndPlay_Implementation()
{
}

void IGIS_EquipmentInterface::OnUnequipped_Implementation()
{
}

// Add default functionality here for any IGIS_EquipmentInterface functions that are not pure virtual.

bool IGIS_EquipmentInterface::IsReplicationManaged_Implementation()
{
	return _getUObject() && _getUObject()->GetClass()->IsChildOf(UGIS_EquipmentInstance::StaticClass());
}
