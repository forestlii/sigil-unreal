// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilArsenalFunctionLibrary.h"

#include "Abilities/GameplayAbility.h"
#include "Equipping/SigilEquipmentSystemComponent.h"
#include "Equipping/SigilWeaponEquipmentInstance.h"
#include "GameFramework/Actor.h"
#include "Weapon/SigilWeaponInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilArsenalFunctionLibrary)

FGameplayTagQuery USigilArsenalFunctionLibrary::MakeAnySlotQuery()
{
	// "No tags of an empty set match" is true for every container, and the query is not IsEmpty().
	FGameplayTagQueryExpression Expression;
	Expression.NoTagsMatch();
	return FGameplayTagQuery::BuildQuery(Expression, TEXT("Sigil.Arsenal.AnySlot"));
}

USigilWeaponEquipmentInstance* USigilArsenalFunctionLibrary::GetActiveWeaponEquipment(const AActor* Actor, const FGameplayTagQuery& SlotQuery)
{
	const USigilEquipmentSystemComponent* Equipment = USigilEquipmentSystemComponent::GetEquipmentSystemComponent(Actor);
	if (!Equipment)
	{
		return nullptr;
	}

	const TArray<UObject*> Active = Equipment->GetActiveEquipments(USigilWeaponEquipmentInstance::StaticClass(), SlotQuery.IsEmpty() ? MakeAnySlotQuery() : SlotQuery);
	return Active.IsEmpty() ? nullptr : Cast<USigilWeaponEquipmentInstance>(Active[0]);
}

AActor* USigilArsenalFunctionLibrary::GetActiveWeaponActor(const AActor* Actor, const FGameplayTagQuery& SlotQuery)
{
	const USigilWeaponEquipmentInstance* Weapon = GetActiveWeaponEquipment(Actor, SlotQuery);
	return Weapon ? Weapon->GetWeaponActor() : nullptr;
}

bool USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions(const AActor* Actor, const FGameplayTagContainer& AbilityTags, const FGameplayTagContainer& SourceTags,
                                                                   const FGameplayTagContainer& TargetTags, TArray<FSigilAbilityAction>& Actions, const FGameplayTagQuery& SlotQuery)
{
	Actions.Reset();
	const USigilWeaponEquipmentInstance* Weapon = GetActiveWeaponEquipment(Actor, SlotQuery);
	return Weapon && Weapon->QueryAbilityActions(AbilityTags, SourceTags, TargetTags, Actions);
}

USigilWeaponEquipmentInstance* USigilArsenalFunctionLibrary::GetWeaponEquipmentOfAbility(const UGameplayAbility* Ability)
{
	return Ability ? Cast<USigilWeaponEquipmentInstance>(Ability->GetCurrentSourceObject()) : nullptr;
}

USigilWeaponEquipmentInstance* USigilArsenalFunctionLibrary::GetWeaponEquipmentOfWeaponActor(AActor* WeaponActor)
{
	if (!IsValid(WeaponActor))
	{
		return nullptr;
	}

	if (WeaponActor->Implements<USigilWeaponInterface>())
	{
		if (USigilWeaponEquipmentInstance* FromSource = Cast<USigilWeaponEquipmentInstance>(ISigilWeaponInterface::Execute_GetSourceObject(WeaponActor)))
		{
			return FromSource;
		}
	}

	if (const USigilEquipmentSystemComponent* Equipment = USigilEquipmentSystemComponent::GetEquipmentSystemComponent(WeaponActor->GetOwner()))
	{
		return Cast<USigilWeaponEquipmentInstance>(Equipment->GetEquipmentInstanceOfActor(WeaponActor));
	}

	return nullptr;
}
