// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AbilitySystem/GCS_GameplayEffectContext.h"

FGameplayEffectContext* FGCS_GameplayEffectContext::Duplicate() const
{
	FGCS_GameplayEffectContext* NewContext = new FGCS_GameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
	{
		// Does a deep copy of the hit result
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

UScriptStruct* FGCS_GameplayEffectContext::GetScriptStruct() const
{
	return StaticStruct();
}

//Reference this:https://www.thegames.dev/?p=62

bool FGCS_GameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);
	enum RepFlag
	{
		REP_AtkDatatable = 8,
		REP_AtkAtkDataTableRowName,
		REP_MAX
	};
	uint16 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (AtkDataTable != nullptr) { RepBits |= 1 << REP_AtkDatatable; }
		if (AtkDataTableRowName != NAME_None) { RepBits |= 1 << REP_AtkAtkDataTableRowName; }
	}

	Ar.SerializeBits(&RepBits, REP_MAX);

	if (RepBits & 1 << REP_AtkDatatable) { Ar << AtkDataTable; }
	if (RepBits & 1 << REP_AtkAtkDataTableRowName) { Ar << AtkDataTableRowName; }

	bOutSuccess = true;
	return true;
}

void FGCS_GameplayEffectContext::SetAttackDefinitionHandle(const FDataTableRowHandle Handle)
{
	if (!Handle.IsNull())
	{
		AtkDataTable = Handle.DataTable;
		AtkDataTableRowName = Handle.RowName;
	}
}

FDataTableRowHandle FGCS_GameplayEffectContext::GetAttackDefinitionHandle() const
{
	FDataTableRowHandle Handle;
	Handle.DataTable = AtkDataTable;
	Handle.RowName = AtkDataTableRowName;
	return Handle;
}
