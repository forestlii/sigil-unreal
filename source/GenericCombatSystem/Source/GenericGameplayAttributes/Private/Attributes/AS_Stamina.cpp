// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Attributes/AS_Stamina.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "GGA_GameplayAttributesHelper.h"
#include "GGA_AttributeSystemComponent.h"


namespace AS_Stamina
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stamina, TEXT("GGF.Attribute.StaminaSet.Stamina"), "Current stamina of an actor.(actor的当前生命值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxStamina, TEXT("GGF.Attribute.StaminaSet.MaxStamina"), "Max stamina value of an actor.(actor的最大生命值)")


    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingHealing, TEXT("GGF.Attribute.StaminaSet.IncomingHealing"), "Incoming healing. This is mapped directly to +Stamina.(即将到来的恢复值，映射为+Stamina)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingDamage, TEXT("GGF.Attribute.StaminaSet.IncomingDamage"), "Incoming damage. This is mapped directly to -Stamina(即将到来的伤害值，映射为-Stamina)")

}

UAS_Stamina::UAS_Stamina()
{

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Stamina::Stamina,GetStaminaAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Stamina::MaxStamina,GetMaxStaminaAttribute());


    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Stamina::IncomingHealing,GetIncomingHealingAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Stamina::IncomingDamage,GetIncomingDamageAttribute());

}

void UAS_Stamina::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Stamina, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxStamina, COND_None, REPNOTIFY_Always);
    
}


void UAS_Stamina::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxStamina());
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void UAS_Stamina::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxStaminaAttribute())
    {
        AdjustAttributeForMaxChange(Stamina, OldValue, NewValue, GetStaminaAttribute());
    }
    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void UAS_Stamina::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        SetStamina(FMath::Clamp(GetStamina(),0,GetMaxStamina()));
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void UAS_Stamina::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute UAS_Stamina::Bp_GetStaminaAttribute()
{
    return ThisClass::GetStaminaAttribute();
}

float UAS_Stamina::Bp_GetStamina() const
{
    return GetStamina();
}

void UAS_Stamina::Bp_SetStamina(float NewValue)
{
    SetStamina(NewValue);
}

void UAS_Stamina::Bp_InitStamina(float NewValue)
{
    InitStamina(NewValue);
}

void UAS_Stamina::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Stamina, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetStaminaAttribute(),GetStamina(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Stamina::Bp_GetMaxStaminaAttribute()
{
    return ThisClass::GetMaxStaminaAttribute();
}

float UAS_Stamina::Bp_GetMaxStamina() const
{
    return GetMaxStamina();
}

void UAS_Stamina::Bp_SetMaxStamina(float NewValue)
{
    SetMaxStamina(NewValue);
}

void UAS_Stamina::Bp_InitMaxStamina(float NewValue)
{
    InitMaxStamina(NewValue);
}

void UAS_Stamina::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxStamina, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxStaminaAttribute(),GetMaxStamina(),OldValue.GetCurrentValue());
        }
    }    
}





FGameplayAttribute UAS_Stamina::Bp_GetIncomingHealingAttribute()
{
    return ThisClass::GetIncomingHealingAttribute();
}

float UAS_Stamina::Bp_GetIncomingHealing() const
{
    return GetIncomingHealing();
}

void UAS_Stamina::Bp_SetIncomingHealing(float NewValue)
{
    SetIncomingHealing(NewValue);
}



FGameplayAttribute UAS_Stamina::Bp_GetIncomingDamageAttribute()
{
    return ThisClass::GetIncomingDamageAttribute();
}

float UAS_Stamina::Bp_GetIncomingDamage() const
{
    return GetIncomingDamage();
}

void UAS_Stamina::Bp_SetIncomingDamage(float NewValue)
{
    SetIncomingDamage(NewValue);
}

