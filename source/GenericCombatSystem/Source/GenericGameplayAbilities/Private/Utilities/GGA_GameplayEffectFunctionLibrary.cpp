// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utilities/GGA_GameplayEffectFunctionLibrary.h"
#include "GameplayEffect.h"
#include "GGA_GameplayEffectContext.h"


// UGameplayEffect* UGGA_GameplayEffectFunctionLibrary::MakeRuntimeGameplayEffect(FString UniqueName, EGameplayEffectDurationType DurationPolicy, TArray<FGameplayModifierInfo> AttributeModifiers)
// {
// 	UGameplayEffect* GameplayEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName("RuntimeGE_" + GetNameSafe(this) + UniqueName));
// 	GameplayEffect->DurationPolicy = DurationPolicy;
// 	GameplayEffect->Modifiers = AttributeModifiers;
// 	return GameplayEffect;
// }

float UGGA_GameplayEffectFunctionLibrary::GetSetByCallerMagnitudeByTag(FGameplayEffectSpecHandle SpecHandle, FGameplayTag DataTag, bool WarnIfNotFound, float DefaultIfNotFound)
{
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	if (Spec)
	{
		return Spec->GetSetByCallerMagnitude(DataTag, WarnIfNotFound, DefaultIfNotFound);
	}
	return 0.0f;
}

float UGGA_GameplayEffectFunctionLibrary::GetSetByCallerMagnitudeByName(FGameplayEffectSpecHandle SpecHandle, FName DataName)
{
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	if (Spec)
	{
		return Spec->GetSetByCallerMagnitude(DataName, false);
	}

	return 0.0f;
}

bool UGGA_GameplayEffectFunctionLibrary::IsActiveGameplayEffectHandleValid(FActiveGameplayEffectHandle Handle)
{
	return Handle.IsValid();
}

void UGGA_GameplayEffectFunctionLibrary::GetOwnedGameplayTags(FGameplayEffectContextHandle EffectContext, FGameplayTagContainer& ActorTagContainer, FGameplayTagContainer& SpecTagContainer)
{
	return EffectContext.GetOwnedGameplayTags(ActorTagContainer, SpecTagContainer);
}

void UGGA_GameplayEffectFunctionLibrary::AddInstigator(FGameplayEffectContextHandle EffectContext, AActor* InInstigator, AActor* InEffectCauser)
{
	EffectContext.AddInstigator(InInstigator, InEffectCauser);
}

void UGGA_GameplayEffectFunctionLibrary::SetEffectCauser(FGameplayEffectContextHandle EffectContext, AActor* InEffectCauser)
{
	EffectContext.AddInstigator(EffectContext.GetInstigator(), InEffectCauser);
}

void UGGA_GameplayEffectFunctionLibrary::SetAbility(FGameplayEffectContextHandle EffectContext, const UGameplayAbility* InGameplayAbility)
{
	EffectContext.SetAbility(InGameplayAbility);
}

const UGameplayAbility* UGGA_GameplayEffectFunctionLibrary::GetAbilityCDO(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetAbility();
}

const UGameplayAbility* UGGA_GameplayEffectFunctionLibrary::GetAbilityInstance(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetAbilityInstance_NotReplicated();
}

int32 UGGA_GameplayEffectFunctionLibrary::GetAbilityLevel(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetAbilityLevel();
}

void UGGA_GameplayEffectFunctionLibrary::AddSourceObject(FGameplayEffectContextHandle EffectContext, const UObject* NewSourceObject)
{
	if (NewSourceObject)
	{
		EffectContext.AddSourceObject(NewSourceObject);
	}
}

bool UGGA_GameplayEffectFunctionLibrary::HasOrigin(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.HasOrigin();
}

UAbilitySystemComponent* UGGA_GameplayEffectFunctionLibrary::GetInstigatorAbilitySystemComponent(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetInstigatorAbilitySystemComponent();
}

UAbilitySystemComponent* UGGA_GameplayEffectFunctionLibrary::GetOriginalInstigatorAbilitySystemComponent(FGameplayEffectContextHandle EffectContext)
{
	return EffectContext.GetOriginalInstigatorAbilitySystemComponent();
}

// bool UGGA_GameplayEffectFunctionLibrary::HasPayload(FGameplayEffectContextHandle EffectContext, const UScriptStruct* PayloadType)
// {
// 	if (FGGA_GameplayEffectContext* Context = static_cast<FGGA_GameplayEffectContext*>(EffectContext.Get()))
// 	{
// 		return Context->HasPayload(PayloadType);
// 	}
// 	return false;
// }
//
// bool UGGA_GameplayEffectFunctionLibrary::SetPayload(FGameplayEffectContextHandle EffectContext, const FInstancedStruct& NewPayload)
// {
// 	if (FGGA_GameplayEffectContext* Context = static_cast<FGGA_GameplayEffectContext*>(EffectContext.Get()))
// 	{
// 		return Context->SetPayload(NewPayload);
// 	}
// 	return false;
// }
//
// FInstancedStruct UGGA_GameplayEffectFunctionLibrary::GetPayload(FGameplayEffectContextHandle EffectContext, const UScriptStruct* PayloadType, bool& bValid)
// {
// 	if (FGGA_GameplayEffectContext* Context = static_cast<FGGA_GameplayEffectContext*>(EffectContext.Get()))
// 	{
// 		return Context->GetPayload(PayloadType, bValid);
// 	}
// 	bValid = false;
// 	return FInstancedStruct();
// }
