// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilAbilitySystemGlobals.h"

#include "GameplayEffect.h"
#include "SigilGasLogChannels.h"

void USigilAbilitySystemGlobals::GlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent)
{
	for (TScriptInterface Receiver : Receivers)
	{
		Receiver->ReceiveGlobalPreGameplayEffectSpecApply(Spec, AbilitySystemComponent);
	}
}

const UAbilitySystemGlobals* USigilAbilitySystemGlobals::GetAbilitySystemGlobals()
{
	if (const UAbilitySystemGlobals* ASG = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals())
	{
		return ASG;
	}

	return nullptr;
}

const UAbilitySystemGlobals* USigilAbilitySystemGlobals::GetTypedAbilitySystemGloabls(TSubclassOf<UAbilitySystemGlobals> DesiredClass)
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

void USigilAbilitySystemGlobals::RegisterEventReceiver(TScriptInterface<ISigilAbilitySystemGlobalsEventReceiver> NewReceiver)
{
	USigilAbilitySystemGlobals* Globals = dynamic_cast<USigilAbilitySystemGlobals*>(&Get());

	if (Globals != nullptr)
	{
		if (NewReceiver != nullptr && IsValid(NewReceiver.GetObject()) && !Globals->Receivers.Contains(NewReceiver))
		{
			Globals->Receivers.Add(NewReceiver);
			UE_LOG(LogSigilAbilitySystem, VeryVerbose, TEXT("RegisterEventReceiver:%s"), *NewReceiver.GetObject()->GetName());
		}
	}
}

void USigilAbilitySystemGlobals::UnregisterEventReceiver(TScriptInterface<ISigilAbilitySystemGlobalsEventReceiver> NewReceiver)
{
	USigilAbilitySystemGlobals* Globals = dynamic_cast<USigilAbilitySystemGlobals*>(&Get());

	if (Globals != nullptr)
	{
		if (NewReceiver != nullptr && IsValid(NewReceiver.GetObject()) && Globals->Receivers.Contains(NewReceiver))
		{
			Globals->Receivers.Remove(NewReceiver);
			UE_LOG(LogSigilAbilitySystem, VeryVerbose, TEXT("UnregisterEventReceiver:%s"), *NewReceiver.GetObject()->GetName());
		}
	}
}

TArray<UCurveTable*> USigilAbilitySystemGlobals::GetAttributeDefaultsTables() const
{
	return GlobalAttributeDefaultsTables;
}

void USigilAbilitySystemGlobals::InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystem, const FSigilAttributeGroupName& GroupName, int32 Level, bool bInitialInit) const
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
		UE_LOG(LogSigilAbilitySystem, Warning, TEXT("You don't have any GlobalAttributeSetDefaultsTableNames configured in your AbilitySystemGlobals setting."))
	}
}

void USigilAbilitySystemGlobals::ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute& InAttribute, const FSigilAttributeGroupName& GroupName, int32 Level) const
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
		UE_LOG(LogSigilAbilitySystem, Warning, TEXT("You don't have any GlobalAttributeSetDefaultsTableNames configured in your AbilitySystemGlobals setting."))
	}
}


void ISigilAbilitySystemGlobalsEventReceiver::ReceiveGlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent)
{
	OnGlobalPreGameplayEffectSpecApply(Spec, AbilitySystemComponent);

	FGameplayTagContainer DynamicTags;
	Execute_OnGlobalPreGameplayEffectSpecApply_Bp(_getUObject(), Spec, AbilitySystemComponent, DynamicTags);
	if (!DynamicTags.IsEmpty())
	{
		Spec.AppendDynamicAssetTags(DynamicTags);
	}
}
