// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Abilities/SigilAbilityCost_ItemIntegerAttribute.h"

#include "Equipping/SigilWeaponEquipmentInstance.h"
#include "GameFramework/Pawn.h"
#include "Items/SigilItemInstance.h"
#include "SigilArsenalTags.h"

namespace
{
USigilWeaponEquipmentInstance* ResolveWeapon(const UGameplayAbility* Ability,
	FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	// 使用本次检查的 Spec，避免激活前或同类技能多来源时读错 CurrentSpec。
	return IsValid(Ability) && ActorInfo
		? Cast<USigilWeaponEquipmentInstance>(Ability->GetSourceObject(Handle, ActorInfo))
		: nullptr;
}

USigilItemInstance* ResolveItem(USigilWeaponEquipmentInstance* Weapon)
{
	return IsValid(Weapon) ? ISigilEquipmentInterface::Execute_GetSourceItem(Weapon) : nullptr;
}
}

USigilAbilityCost_ItemIntegerAttribute::USigilAbilityCost_ItemIntegerAttribute()
	: Tag(SigilArsenalTags::Ammo_Magazine), FailureTag(SigilArsenalTags::Ability_Fail_Ammo)
{
}

bool USigilAbilityCost_ItemIntegerAttribute::CheckCost(const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	const USigilItemInstance* Item = ResolveItem(ResolveWeapon(Ability, Handle, ActorInfo));
	const bool bCanPay = Tag.IsValid() && Quantity > 0 && IsValid(Item)
		&& Item->HasIntegerAttribute(Tag) && Item->GetIntegerAttribute(Tag) >= Quantity;
	if (!bCanPay && OptionalRelevantTags && FailureTag.IsValid())
	{
		OptionalRelevantTags->AddTag(FailureTag);
	}
	return bCanPay;
}

void USigilAbilityCost_ItemIntegerAttribute::ApplyCost(const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo)
{
	USigilWeaponEquipmentInstance* Weapon = ResolveWeapon(Ability, Handle, ActorInfo);
	const APawn* OwningPawn = IsValid(Weapon) ? ISigilEquipmentInterface::Execute_GetOwningPawn(Weapon) : nullptr;
	if (!IsValid(OwningPawn) || !OwningPawn->HasAuthority() || !Tag.IsValid() || Quantity <= 0)
	{
		return;
	}

	USigilItemInstance* Item = ResolveItem(Weapon);
	if (IsValid(Item) && Item->HasIntegerAttribute(Tag))
	{
		const int32 Available = Item->GetIntegerAttribute(Tag);
		// Apply 可能被直接调用，或此前的 Cost 已改写余额；不允许扣成负数。
		if (Available >= Quantity)
		{
			Item->SetIntegerAttribute(Tag, Available - Quantity);
		}
	}
}
