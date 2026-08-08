// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utilities/SigilGameplayEffectFunctionLibrary.h"
#include "GameplayEffect.h"


float USigilGameplayEffectFunctionLibrary::GetSetByCallerMagnitudeByTag(FGameplayEffectSpecHandle SpecHandle, FGameplayTag DataTag, bool WarnIfNotFound, float DefaultIfNotFound)
{
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	if (Spec)
	{
		return Spec->GetSetByCallerMagnitude(DataTag, WarnIfNotFound, DefaultIfNotFound);
	}
	return 0.0f;
}

float USigilGameplayEffectFunctionLibrary::GetSetByCallerMagnitudeByName(FGameplayEffectSpecHandle SpecHandle, FName DataName)
{
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	if (Spec)
	{
		return Spec->GetSetByCallerMagnitude(DataName, false);
	}

	return 0.0f;
}

bool USigilGameplayEffectFunctionLibrary::IsActiveGameplayEffectHandleValid(FActiveGameplayEffectHandle Handle)
{
	return Handle.IsValid();
}

void USigilGameplayEffectFunctionLibrary::GetOwnedGameplayTags(FGameplayEffectContextHandle EffectContext, FGameplayTagContainer& ActorTagContainer, FGameplayTagContainer& SpecTagContainer)
{
	return EffectContext.GetOwnedGameplayTags(ActorTagContainer, SpecTagContainer);
}

void USigilGameplayEffectFunctionLibrary::AddInstigator(FGameplayEffectContextHandle EffectContext, AActor* InInstigator, AActor* InEffectCauser)
{
	EffectContext.AddInstigator(InInstigator, InEffectCauser);
}

void USigilGameplayEffectFunctionLibrary::SetEffectCauser(FGameplayEffectContextHandle EffectContext, AActor* InEffectCauser)
{
	EffectContext.AddInstigator(EffectContext.GetInstigator(), InEffectCauser);
}

void USigilGameplayEffectFunctionLibrary::SetAbility(FGameplayEffectContextHandle EffectContext, const UGameplayAbility* InGameplayAbility)
{
	EffectContext.SetAbility(InGameplayAbility);
}

const UGameplayAbility* USigilGameplayEffectFunctionLibrary::GetAbilityCDO(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetAbility();
}

const UGameplayAbility* USigilGameplayEffectFunctionLibrary::GetAbilityInstance(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetAbilityInstance_NotReplicated();
}

int32 USigilGameplayEffectFunctionLibrary::GetAbilityLevel(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetAbilityLevel();
}

void USigilGameplayEffectFunctionLibrary::AddSourceObject(FGameplayEffectContextHandle EffectContext, const UObject* NewSourceObject)
{
	if (NewSourceObject)
	{
		EffectContext.AddSourceObject(NewSourceObject);
	}
}

bool USigilGameplayEffectFunctionLibrary::HasOrigin(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.HasOrigin();
}

UAbilitySystemComponent* USigilGameplayEffectFunctionLibrary::GetInstigatorAbilitySystemComponent(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetInstigatorAbilitySystemComponent();
}

UAbilitySystemComponent* USigilGameplayEffectFunctionLibrary::GetOriginalInstigatorAbilitySystemComponent(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetOriginalInstigatorAbilitySystemComponent();
}
