// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AbilitySystem/SigilGameplayEffectContext.h"

FGameplayEffectContext* FSigilGameplayEffectContext::Duplicate() const
{
	FSigilGameplayEffectContext* NewContext = new FSigilGameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
	{
		// Does a deep copy of the hit result
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

UScriptStruct* FSigilGameplayEffectContext::GetScriptStruct() const
{
	return StaticStruct();
}

//Reference this:https://www.thegames.dev/?p=62

bool FSigilGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
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

void FSigilGameplayEffectContext::SetAttackDefinitionHandle(const FDataTableRowHandle Handle)
{
	if (!Handle.IsNull())
	{
		AtkDataTable = Handle.DataTable;
		AtkDataTableRowName = Handle.RowName;
	}
}

FDataTableRowHandle FSigilGameplayEffectContext::GetAttackDefinitionHandle() const
{
	FDataTableRowHandle Handle;
	Handle.DataTable = AtkDataTable;
	Handle.RowName = AtkDataTableRowName;
	return Handle;
}
