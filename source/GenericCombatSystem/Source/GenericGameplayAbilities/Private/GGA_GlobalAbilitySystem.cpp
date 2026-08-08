// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GGA_GlobalAbilitySystem.h"
#include "GGA_AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"


void FGGA_GlobalAppliedAbilityList::AddToASC(TSubclassOf<UGameplayAbility> Ability, UGGA_AbilitySystemComponent* ASC)
{
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
	FGameplayAbilitySpec AbilitySpec(AbilityCDO);
	const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);
	Handles.Add(ASC, AbilitySpecHandle);
}

void FGGA_GlobalAppliedAbilityList::RemoveFromASC(UGGA_AbilitySystemComponent* ASC)
{
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		ASC->ClearAbility(*SpecHandle);
		Handles.Remove(ASC);
	}
}

void FGGA_GlobalAppliedAbilityList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->ClearAbility(KVP.Value);
		}
	}
	Handles.Empty();
}



void FGGA_GlobalAppliedEffectList::AddToASC(TSubclassOf<UGameplayEffect> Effect, UGGA_AbilitySystemComponent* ASC)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	const UGameplayEffect* GameplayEffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffectCDO, /*Level=*/ 1, ASC->MakeEffectContext());
	Handles.Add(ASC, GameplayEffectHandle);
}

void FGGA_GlobalAppliedEffectList::RemoveFromASC(UGGA_AbilitySystemComponent* ASC)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(ASC))
	{
		ASC->RemoveActiveGameplayEffect(*EffectHandle);
		Handles.Remove(ASC);
	}
}

void FGGA_GlobalAppliedEffectList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->RemoveActiveGameplayEffect(KVP.Value);
		}
	}
	Handles.Empty();
}

UGGA_GlobalAbilitySystem::UGGA_GlobalAbilitySystem()
{
}

void UGGA_GlobalAbilitySystem::ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && (!AppliedAbilities.Contains(Ability)))
	{
		FGGA_GlobalAppliedAbilityList& Entry = AppliedAbilities.Add(Ability);		
		for (UGGA_AbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.AddToASC(Ability, ASC);
		}
	}
}

void UGGA_GlobalAbilitySystem::ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && (!AppliedEffects.Contains(Effect)))
	{
		FGGA_GlobalAppliedEffectList& Entry = AppliedEffects.Add(Effect);
		for (UGGA_AbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.AddToASC(Effect, ASC);
		}
	}
}

void UGGA_GlobalAbilitySystem::RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && AppliedAbilities.Contains(Ability))
	{
		FGGA_GlobalAppliedAbilityList& Entry = AppliedAbilities[Ability];
		Entry.RemoveFromAll();
		AppliedAbilities.Remove(Ability);
	}
}

void UGGA_GlobalAbilitySystem::RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && AppliedEffects.Contains(Effect))
	{
		FGGA_GlobalAppliedEffectList& Entry = AppliedEffects[Effect];
		Entry.RemoveFromAll();
		AppliedEffects.Remove(Effect);
	}
}

void UGGA_GlobalAbilitySystem::RegisterASC(UGGA_AbilitySystemComponent* ASC)
{
	check(ASC);

	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.AddToASC(Entry.Key, ASC);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.AddToASC(Entry.Key, ASC);
	}

	RegisteredASCs.AddUnique(ASC);
}

void UGGA_GlobalAbilitySystem::UnregisterASC(UGGA_AbilitySystemComponent* ASC)
{
	check(ASC);
	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.RemoveFromASC(ASC);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.RemoveFromASC(ASC);
	}

	RegisteredASCs.Remove(ASC);
}
