// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Fragments/GIS_ItemFragment.h"

#include "Items/GIS_ItemStack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_ItemFragment)


bool UGIS_ItemFragment::IsMixinDataSerializable() const
{
	return IsStateSerializable();
}

TObjectPtr<const UScriptStruct> UGIS_ItemFragment::GetCompatibleMixinDataType() const
{
	return GetCompatibleStateType();
}

bool UGIS_ItemFragment::MakeDefaultMixinData(FInstancedStruct& DefaultState) const
{
	return MakeDefaultState(DefaultState);
}

bool UGIS_ItemFragment::MakeDefaultState_Implementation(FInstancedStruct& DefaultState) const
{
	return false;
}

const UScriptStruct* UGIS_ItemFragment::GetCompatibleStateType_Implementation() const
{
	return nullptr;
}

bool UGIS_ItemFragment::IsStateSerializable_Implementation() const
{
	return false;
}
