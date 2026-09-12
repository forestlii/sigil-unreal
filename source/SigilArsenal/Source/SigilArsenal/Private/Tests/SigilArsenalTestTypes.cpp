// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilArsenalTestTypes.h"

#include "Abilities/SigilAbilityCost_ItemIntegerAttribute.h"
#include "Components/SkeletalMeshComponent.h"
#include "SigilArsenalTags.h"

namespace SigilArsenalTestTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemGun, "Sigil.Test.Arsenal.Item.Gun", "Automation-only item tag for weapon loadout tests.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SlotWeaponGroup, "Sigil.Test.Arsenal.Slots.Weapon", "Automation-only weapon slot group.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SlotPrimary, "Sigil.Test.Arsenal.Slots.Weapon.Primary", "Automation-only primary weapon slot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SlotSecondary, "Sigil.Test.Arsenal.Slots.Weapon.Secondary", "Automation-only secondary weapon slot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityFire, "Sigil.Test.Arsenal.Ability.Fire", "Automation-only ability tag resolved through the weapon action set.");
}

ASigilArsenalTestPawn::ASigilArsenalTestPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;

	MainMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MainMesh"));
	RootComponent = MainMesh;
	AbilitySystem = CreateDefaultSubobject<USigilAbilitySystemComponent>(TEXT("AbilitySystem"));
	Inventory = CreateDefaultSubobject<USigilArsenalTestInventoryComponent>(TEXT("Inventory"));
	Equipment = CreateDefaultSubobject<USigilArsenalTestEquipmentComponent>(TEXT("Equipment"));
}

USigilArsenalTestFireAbility::USigilArsenalTestFireAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bRequireSourceObjectActive = true;
}

void USigilArsenalTestFireAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	++ActivationCount;
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

USigilArsenalTestAbilitySet::USigilArsenalTestAbilitySet()
{
	FSigilAbilitySet_GameplayAbility& Entry = GrantedGameplayAbilities.AddDefaulted_GetRef();
	Entry.Ability = USigilArsenalTestFireAbility::StaticClass();
	Entry.AbilityLevel = 1;
}

void USigilArsenalTestAmmoAttributes::SetMagazineForTest(int32 Amount)
{
	InitialIntegerAttributes.Reset();
	InitialIntegerAttributes.Emplace(SigilArsenalTags::Ammo_Magazine, Amount);
	// 测试运行时创建片段，需同步编辑器通常在 PreSave 生成的缓存。
	IntegerAttributeMap.Reset();
	IntegerAttributeMap.Add(SigilArsenalTags::Ammo_Magazine, Amount);
}

USigilArsenalTestAmmoFireAbility::USigilArsenalTestAmmoFireAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bRequireSourceObjectActive = true;
	AdditionalCosts.Add(CreateDefaultSubobject<USigilAbilityCost_ItemIntegerAttribute>(TEXT("AmmoCost")));
}

USigilAbilityCost_ItemIntegerAttribute* USigilArsenalTestAmmoFireAbility::GetAmmoCostForTest() const
{
	return AdditionalCosts.IsEmpty() ? nullptr : Cast<USigilAbilityCost_ItemIntegerAttribute>(AdditionalCosts[0]);
}

void USigilArsenalTestAmmoFireAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	++ActivationCount;
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

USigilArsenalTestAmmoAbilitySet::USigilArsenalTestAmmoAbilitySet()
{
	FSigilAbilitySet_GameplayAbility& Entry = GrantedGameplayAbilities.AddDefaulted_GetRef();
	Entry.Ability = USigilArsenalTestAmmoFireAbility::StaticClass();
	Entry.AbilityLevel = 1;
}
