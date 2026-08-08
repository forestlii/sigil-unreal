// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "AbilitySystem/Attributes/SigilCombatSet.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "SigilGameplayAttributesHelper.h"
#include "SigilAttributeSystemComponent.h"


namespace SigilCombatSet
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage, TEXT("Sigil.Attribute.CombatSet.Damage"), "The damage that will apply to target")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(DamageNegation, TEXT("Sigil.Attribute.CombatSet.DamageNegation"), "The damage reduction(percentage) for incoming health damage")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GuardDamageNegation, TEXT("Sigil.Attribute.CombatSet.GuardDamageNegation"), "The damage reduction(percentage) for incoming health damage while guarding")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(StaminaDamage, TEXT("Sigil.Attribute.CombatSet.StaminaDamage"), "The stamina damage that will apply to target")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(StaminaDamageNegation, TEXT("Sigil.Attribute.CombatSet.StaminaDamageNegation"), "The damage reduction(percentage) for incoming stamina damage")


}

USigilCombatSet::USigilCombatSet()
{

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilCombatSet::Damage,GetDamageAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilCombatSet::DamageNegation,GetDamageNegationAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilCombatSet::GuardDamageNegation,GetGuardDamageNegationAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilCombatSet::StaminaDamage,GetStaminaDamageAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilCombatSet::StaminaDamageNegation,GetStaminaDamageNegationAttribute());


}

void USigilCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Damage, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, DamageNegation, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, GuardDamageNegation, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, StaminaDamage, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, StaminaDamageNegation, COND_None, REPNOTIFY_Always);
    
}


void USigilCombatSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    

    

    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void USigilCombatSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    

    

    

    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void USigilCombatSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    

    

    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void USigilCombatSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute USigilCombatSet::Bp_GetDamageAttribute()
{
    return ThisClass::GetDamageAttribute();
}

float USigilCombatSet::Bp_GetDamage() const
{
    return GetDamage();
}

void USigilCombatSet::Bp_SetDamage(float NewValue)
{
    SetDamage(NewValue);
}

void USigilCombatSet::Bp_InitDamage(float NewValue)
{
    InitDamage(NewValue);
}

void USigilCombatSet::OnRep_Damage(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Damage, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetDamageAttribute(),GetDamage(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilCombatSet::Bp_GetDamageNegationAttribute()
{
    return ThisClass::GetDamageNegationAttribute();
}

float USigilCombatSet::Bp_GetDamageNegation() const
{
    return GetDamageNegation();
}

void USigilCombatSet::Bp_SetDamageNegation(float NewValue)
{
    SetDamageNegation(NewValue);
}

void USigilCombatSet::Bp_InitDamageNegation(float NewValue)
{
    InitDamageNegation(NewValue);
}

void USigilCombatSet::OnRep_DamageNegation(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, DamageNegation, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetDamageNegationAttribute(),GetDamageNegation(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilCombatSet::Bp_GetGuardDamageNegationAttribute()
{
    return ThisClass::GetGuardDamageNegationAttribute();
}

float USigilCombatSet::Bp_GetGuardDamageNegation() const
{
    return GetGuardDamageNegation();
}

void USigilCombatSet::Bp_SetGuardDamageNegation(float NewValue)
{
    SetGuardDamageNegation(NewValue);
}

void USigilCombatSet::Bp_InitGuardDamageNegation(float NewValue)
{
    InitGuardDamageNegation(NewValue);
}

void USigilCombatSet::OnRep_GuardDamageNegation(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, GuardDamageNegation, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetGuardDamageNegationAttribute(),GetGuardDamageNegation(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilCombatSet::Bp_GetStaminaDamageAttribute()
{
    return ThisClass::GetStaminaDamageAttribute();
}

float USigilCombatSet::Bp_GetStaminaDamage() const
{
    return GetStaminaDamage();
}

void USigilCombatSet::Bp_SetStaminaDamage(float NewValue)
{
    SetStaminaDamage(NewValue);
}

void USigilCombatSet::Bp_InitStaminaDamage(float NewValue)
{
    InitStaminaDamage(NewValue);
}

void USigilCombatSet::OnRep_StaminaDamage(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, StaminaDamage, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetStaminaDamageAttribute(),GetStaminaDamage(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilCombatSet::Bp_GetStaminaDamageNegationAttribute()
{
    return ThisClass::GetStaminaDamageNegationAttribute();
}

float USigilCombatSet::Bp_GetStaminaDamageNegation() const
{
    return GetStaminaDamageNegation();
}

void USigilCombatSet::Bp_SetStaminaDamageNegation(float NewValue)
{
    SetStaminaDamageNegation(NewValue);
}

void USigilCombatSet::Bp_InitStaminaDamageNegation(float NewValue)
{
    InitStaminaDamageNegation(NewValue);
}

void USigilCombatSet::OnRep_StaminaDamageNegation(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, StaminaDamageNegation, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetStaminaDamageNegationAttribute(),GetStaminaDamageNegation(),OldValue.GetCurrentValue());
        }
    }    
}



