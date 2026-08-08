// // Copyright 2025 RedMoonGames All Rights Reserved.
//
//
// #include "GIS_EquipItemInstance.h"
// #include "GIS_InventorySystemSettings.h"
//
// #include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_EquipItemInstance)
//
// TMap<FGameplayTag, float> UGIS_EquipItemInstance::GetAdditionAttributes() const
// {
// 	return AdditionAttributes;
// }
//
// void UGIS_EquipItemInstance::OverrideAdditionAttribute(FGameplayTag AttributeTag, float Value)
// {
// 	if (HasFloatAttribute(AttributeTag))
// 	{
// 		AdditionAttributes.Emplace(AttributeTag, Value);
// 	}
// }
//
// TMap<FGameplayTag, float> UGIS_EquipItemInstance::GetMultiplierAttributes() const
// {
// 	return MultiplierAttributes;
// }
//
// void UGIS_EquipItemInstance::OverrideMultiplierAttribute(FGameplayTag AttributeTag, float Value)
// {
// 	//不会强化不存在的属性
// 	if (HasFloatAttribute(AttributeTag))
// 	{
// 		MultiplierAttributes.Emplace(AttributeTag, Value);
// 	}
// }
//
// int32 UGIS_EquipItemInstance::GetEnhancedLevel() const
// {
// 	return EnhancedLevel;
// }
//
// int32 UGIS_EquipItemInstance::GetMaxEnhancedNumber() const
// {
// 	return MaxEnhancedNumber;
// }
//
// int32 UGIS_EquipItemInstance::GetCurrentEnhancedNumber() const
// {
// 	return CurrentEnhancedNumber;
// }
//
// void UGIS_EquipItemInstance::SetEnhancedLevel(int32 Value)
// {
// 	EnhancedLevel = Value;
// }
//
// void UGIS_EquipItemInstance::SetMaxEnhancedNumber(int32 Value)
// {
// 	MaxEnhancedNumber = Value;
// }
//
// void UGIS_EquipItemInstance::SetCurrentEnhancedNumber(int32 Value)
// {
// 	CurrentEnhancedNumber = Value;
// }
//
//
// float UGIS_EquipItemInstance::GetFloatAttribute(FGameplayTag AttributeTag) const
// {
// 	const float Base = Super::GetFloatAttribute(AttributeTag);
// 	const float Addition = AdditionAttributes.Contains(AttributeTag) ? AdditionAttributes[AttributeTag] : 0;
// 	const float Multiplier = MultiplierAttributes.Contains(AttributeTag) ? MultiplierAttributes[AttributeTag] : 0;
// 	return Base + Addition + Base * Multiplier;
// }
