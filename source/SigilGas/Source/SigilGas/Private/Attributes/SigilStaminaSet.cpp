// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Attributes/SigilStaminaSet.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "SigilGameplayAttributesHelper.h"
#include "SigilAttributeSystemComponent.h"


namespace SigilStaminaSet
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stamina, TEXT("Sigil.Attribute.StaminaSet.Stamina"), "Current stamina of an actor.(actor的当前生命值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxStamina, TEXT("Sigil.Attribute.StaminaSet.MaxStamina"), "Max stamina value of an actor.(actor的最大生命值)")


    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingHealing, TEXT("Sigil.Attribute.StaminaSet.IncomingHealing"), "Incoming healing. This is mapped directly to +Stamina.(即将到来的恢复值，映射为+Stamina)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingDamage, TEXT("Sigil.Attribute.StaminaSet.IncomingDamage"), "Incoming damage. This is mapped directly to -Stamina(即将到来的伤害值，映射为-Stamina)")

}

USigilStaminaSet::USigilStaminaSet()
{

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilStaminaSet::Stamina,GetStaminaAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilStaminaSet::MaxStamina,GetMaxStaminaAttribute());


    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilStaminaSet::IncomingHealing,GetIncomingHealingAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilStaminaSet::IncomingDamage,GetIncomingDamageAttribute());

}

void USigilStaminaSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Stamina, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxStamina, COND_None, REPNOTIFY_Always);
    
}


void USigilStaminaSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxStamina());
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void USigilStaminaSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxStaminaAttribute())
    {
        AdjustAttributeForMaxChange(Stamina, OldValue, NewValue, GetStaminaAttribute());
    }
    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void USigilStaminaSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        SetStamina(FMath::Clamp(GetStamina(),0,GetMaxStamina()));
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void USigilStaminaSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
                                                   const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}



FGameplayAttribute USigilStaminaSet::Bp_GetStaminaAttribute()
{
    return ThisClass::GetStaminaAttribute();
}

float USigilStaminaSet::Bp_GetStamina() const
{
    return GetStamina();
}

void USigilStaminaSet::Bp_SetStamina(float NewValue)
{
    SetStamina(NewValue);
}

void USigilStaminaSet::Bp_InitStamina(float NewValue)
{
    InitStamina(NewValue);
}

void USigilStaminaSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Stamina, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetStaminaAttribute(),GetStamina(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilStaminaSet::Bp_GetMaxStaminaAttribute()
{
    return ThisClass::GetMaxStaminaAttribute();
}

float USigilStaminaSet::Bp_GetMaxStamina() const
{
    return GetMaxStamina();
}

void USigilStaminaSet::Bp_SetMaxStamina(float NewValue)
{
    SetMaxStamina(NewValue);
}

void USigilStaminaSet::Bp_InitMaxStamina(float NewValue)
{
    InitMaxStamina(NewValue);
}

void USigilStaminaSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxStamina, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxStaminaAttribute(),GetMaxStamina(),OldValue.GetCurrentValue());
        }
    }    
}





FGameplayAttribute USigilStaminaSet::Bp_GetIncomingHealingAttribute()
{
    return ThisClass::GetIncomingHealingAttribute();
}

float USigilStaminaSet::Bp_GetIncomingHealing() const
{
    return GetIncomingHealing();
}

void USigilStaminaSet::Bp_SetIncomingHealing(float NewValue)
{
    SetIncomingHealing(NewValue);
}



FGameplayAttribute USigilStaminaSet::Bp_GetIncomingDamageAttribute()
{
    return ThisClass::GetIncomingDamageAttribute();
}

float USigilStaminaSet::Bp_GetIncomingDamage() const
{
    return GetIncomingDamage();
}

void USigilStaminaSet::Bp_SetIncomingDamage(float NewValue)
{
    SetIncomingDamage(NewValue);
}

