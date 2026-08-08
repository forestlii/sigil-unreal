// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GGA_AbilitySystemGlobals.h"

#include "GameplayEffect.h"
#include "GGA_LogChannels.h"

void UGGA_AbilitySystemGlobals::GlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent)
{
	for (TScriptInterface Receiver : Receivers)
	{
		Receiver->ReceiveGlobalPreGameplayEffectSpecApply(Spec, AbilitySystemComponent);
	}
}

const UAbilitySystemGlobals* UGGA_AbilitySystemGlobals::GetAbilitySystemGlobals()
{
	if (const UAbilitySystemGlobals* ASG = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals())
	{
		return ASG;
	}

	return nullptr;
}

const UAbilitySystemGlobals* UGGA_AbilitySystemGlobals::GetTypedAbilitySystemGloabls(TSubclassOf<UAbilitySystemGlobals> DesiredClass)
{
	if (UClass* RealClass = DesiredClass)
	{
		if (const UAbilitySystemGlobals* ASG = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals())
		{
			if (ASG->GetClass()->IsChildOf(RealClass))
			{
				return ASG;
			}
		}
	}
	return nullptr;
}

void UGGA_AbilitySystemGlobals::RegisterEventReceiver(TScriptInterface<IGGA_AbilitySystemGlobalsEventReceiver> NewReceiver)
{
	UGGA_AbilitySystemGlobals* Globals = dynamic_cast<UGGA_AbilitySystemGlobals*>(&Get());

	if (Globals != nullptr)
	{
		if (NewReceiver != nullptr && IsValid(NewReceiver.GetObject()) && !Globals->Receivers.Contains(NewReceiver))
		{
			Globals->Receivers.Add(NewReceiver);
			UE_LOG(LogGGA_AbilitySystem, VeryVerbose, TEXT("RegisterEventReceiver:%s"), *NewReceiver.GetObject()->GetName());
		}
	}
}

void UGGA_AbilitySystemGlobals::UnregisterEventReceiver(TScriptInterface<IGGA_AbilitySystemGlobalsEventReceiver> NewReceiver)
{
	UGGA_AbilitySystemGlobals* Globals = dynamic_cast<UGGA_AbilitySystemGlobals*>(&Get());

	if (Globals != nullptr)
	{
		if (NewReceiver != nullptr && IsValid(NewReceiver.GetObject()) && Globals->Receivers.Contains(NewReceiver))
		{
			Globals->Receivers.Remove(NewReceiver);
			UE_LOG(LogGGA_AbilitySystem, VeryVerbose, TEXT("UnregisterEventReceiver:%s"), *NewReceiver.GetObject()->GetName());
		}
	}
}

TArray<UCurveTable*> UGGA_AbilitySystemGlobals::GetAttributeDefaultsTables() const
{
	return GlobalAttributeDefaultsTables;
}

void UGGA_AbilitySystemGlobals::InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystem, const FGGA_AttributeGroupName& GroupName, int32 Level, bool bInitialInit) const
{
	if (GlobalAttributeSetInitter.IsValid())
	{
		if (FAttributeSetInitter* Initter = GetAttributeSetInitter())
		{
			if (GroupName.IsValid())
			{
				Initter->InitAttributeSetDefaults(AbilitySystem, GroupName.GetName(), Level, bInitialInit);
			}
		}
	}
	else
	{
		UE_LOG(LogGGA_AbilitySystem, Warning, TEXT("You don't have any GlobalAttributeSetDefaultsTableNames configured in your AbilitySystemGlobals setting."))
	}
}

void UGGA_AbilitySystemGlobals::ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute& InAttribute, const FGGA_AttributeGroupName& GroupName, int32 Level) const
{
	if (GlobalAttributeSetInitter.IsValid())
	{
		if (FAttributeSetInitter* Initter = GetAttributeSetInitter())
		{
			if (GroupName.IsValid())
			{
				Initter->ApplyAttributeDefault(AbilitySystem, InAttribute, GroupName.GetName(), Level);
			}
		}
	}
	else
	{
		UE_LOG(LogGGA_AbilitySystem, Warning, TEXT("You don't have any GlobalAttributeSetDefaultsTableNames configured in your AbilitySystemGlobals setting."))
	}
}


void IGGA_AbilitySystemGlobalsEventReceiver::ReceiveGlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent)
{
	OnGlobalPreGameplayEffectSpecApply(Spec, AbilitySystemComponent);

	FGameplayTagContainer DynamicTags;
	Execute_OnGlobalPreGameplayEffectSpecApply_Bp(_getUObject(), Spec, AbilitySystemComponent, DynamicTags);
	if (!DynamicTags.IsEmpty())
	{
		Spec.AppendDynamicAssetTags(DynamicTags);
	}
}
