// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Equipping/SigilWeaponEquipmentInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimInstance.h"
#include "CombatFlow/SigilAbilityActionSetSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Fragments/SigilItemFragment_WeaponLoadout.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Items/SigilItemInstance.h"
#include "SigilArsenalLogChannels.h"
#include "Utility/SigilCombatFunctionLibrary.h"
#include "Weapon/SigilWeaponInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilWeaponEquipmentInstance)

void USigilWeaponEquipmentInstance::CacheLoadout()
{
	CachedLoadout = SourceItem ? SourceItem->FindFragmentByClass<USigilItemFragment_WeaponLoadout>() : nullptr;
	CachedAbilitySet = nullptr;
	CachedAbilityActionSet = nullptr;
	CachedAnimLayerClass = nullptr;

	if (!CachedLoadout)
	{
		UE_LOG(LogSigilArsenal, Warning, TEXT("%s: source item [%s] has no WeaponLoadout fragment; nothing will be granted or linked."), *GetName(), *GetNameSafe(SourceItem));
		return;
	}

	if (!CachedLoadout->AbilitySet.IsNull())
	{
		CachedAbilitySet = CachedLoadout->AbilitySet.LoadSynchronous();
	}
	if (!CachedLoadout->AbilityActionSet.IsNull())
	{
		CachedAbilityActionSet = CachedLoadout->AbilityActionSet.LoadSynchronous();
	}
	if (!CachedLoadout->AnimLayerClass.IsNull())
	{
		CachedAnimLayerClass = CachedLoadout->AnimLayerClass.LoadSynchronous();
	}
}

void USigilWeaponEquipmentInstance::OnEquipmentBeginPlay_Implementation()
{
	Super::OnEquipmentBeginPlay_Implementation();

	// Runs on every machine: clients need the action set and the anim layer too.
	CacheLoadout();

	if (CachedLoadout && CachedLoadout->GrantPolicy == ESigilWeaponAbilityGrantPolicy::WhileEquipped)
	{
		GrantAbilities();
	}
}

void USigilWeaponEquipmentInstance::OnEquipmentEndPlay_Implementation()
{
	RevokeAbilities();
	UnlinkAnimLayer();

	Super::OnEquipmentEndPlay_Implementation();
}

void USigilWeaponEquipmentInstance::OnActiveStateChanged_Implementation(bool bNewActiveState)
{
	Super::OnActiveStateChanged_Implementation(bNewActiveState);

	if (bNewActiveState)
	{
		LinkAnimLayer();
	}
	else
	{
		UnlinkAnimLayer();
	}

	if (CachedLoadout && CachedLoadout->GrantPolicy == ESigilWeaponAbilityGrantPolicy::WhileActive)
	{
		if (bNewActiveState)
		{
			GrantAbilities();
		}
		else
		{
			RevokeAbilities();
		}
	}
}

bool USigilWeaponEquipmentInstance::IsAbilitySourceActive_Implementation() const
{
	return ISigilEquipmentInterface::Execute_IsEquipmentActive(this);
}

bool USigilWeaponEquipmentInstance::QueryAbilityActions(const FGameplayTagContainer& AbilityTags, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags,
                                                        TArray<FSigilAbilityAction>& Actions) const
{
	Actions.Reset();
	return CachedAbilityActionSet && CachedAbilityActionSet->SelectBestAbilityActions(SourceTags, TargetTags, AbilityTags, Actions);
}

AActor* USigilWeaponEquipmentInstance::GetWeaponActor() const
{
	for (AActor* Actor : EquipmentActors)
	{
		if (IsValid(Actor) && Actor->Implements<USigilWeaponInterface>())
		{
			return Actor;
		}
	}
	return nullptr;
}

UAbilitySystemComponent* USigilWeaponEquipmentInstance::GetOwnerAbilitySystemComponent() const
{
	return ResolveAbilitySystemComponent();
}

UAbilitySystemComponent* USigilWeaponEquipmentInstance::ResolveAbilitySystemComponent() const
{
	if (!OwningPawn)
	{
		return nullptr;
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningPawn))
	{
		return ASC;
	}

	// PlayerState-hosted ability systems.
	if (APlayerState* PlayerState = OwningPawn->GetPlayerState())
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState);
	}

	return nullptr;
}

USkeletalMeshComponent* USigilWeaponEquipmentInstance::ResolveAnimLayerMesh() const
{
	if (!OwningPawn)
	{
		return nullptr;
	}

	const FName LookupTag = CachedLoadout ? CachedLoadout->AnimLayerMeshLookupTag : NAME_None;
	return USigilCombatFunctionLibrary::GetMainCharacterMeshComponent(OwningPawn, LookupTag);
}

void USigilWeaponEquipmentInstance::SetupEquipmentActors_Implementation(const TArray<AActor*>& InActors) const
{
	Super::SetupEquipmentActors_Implementation(InActors);

	// Weapon actors point back at this instance, so combat code holding a weapon can reach the item and the loadout.
	// ISigilWeaponInterface::SetSourceObject is authority-only and replicated on the weapon actor itself.
	for (AActor* Actor : InActors)
	{
		if (IsValid(Actor) && Actor->Implements<USigilWeaponInterface>())
		{
			ISigilWeaponInterface::Execute_SetSourceObject(Actor, const_cast<USigilWeaponEquipmentInstance*>(this));
		}
	}
}

void USigilWeaponEquipmentInstance::GrantAbilities()
{
	if (bAbilitiesGranted || !CachedAbilitySet || !OwningPawn || !OwningPawn->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent();
	if (!ASC || !ASC->GetOwnerActor())
	{
		UE_LOG(LogSigilArsenal, Warning, TEXT("%s: cannot grant [%s], pawn [%s] has no initialized ability system component."), *GetName(), *GetNameSafe(CachedAbilitySet), *GetNameSafe(OwningPawn));
		return;
	}

	CachedAbilitySet->GiveToAbilitySystem(ASC, &GrantedHandles, this);
	bAbilitiesGranted = true;
}

void USigilWeaponEquipmentInstance::RevokeAbilities()
{
	if (!bAbilitiesGranted)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent())
	{
		GrantedHandles.TakeFromAbilitySystem(ASC);
	}
	GrantedHandles = FSigilAbilitySet_GrantedHandles();
	bAbilitiesGranted = false;
}

void USigilWeaponEquipmentInstance::LinkAnimLayer()
{
	if (!CachedAnimLayerClass || LinkedAnimLayerClass)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = ResolveAnimLayerMesh();
	if (!Mesh)
	{
		return;
	}

	Mesh->LinkAnimClassLayers(CachedAnimLayerClass);
	LinkedAnimLayerClass = CachedAnimLayerClass;
	LinkedAnimLayerMesh = Mesh;
}

void USigilWeaponEquipmentInstance::UnlinkAnimLayer()
{
	if (!LinkedAnimLayerClass)
	{
		return;
	}

	if (USkeletalMeshComponent* Mesh = LinkedAnimLayerMesh.Get())
	{
		Mesh->UnlinkAnimClassLayers(LinkedAnimLayerClass);
	}
	LinkedAnimLayerClass = nullptr;
	LinkedAnimLayerMesh = nullptr;
}
