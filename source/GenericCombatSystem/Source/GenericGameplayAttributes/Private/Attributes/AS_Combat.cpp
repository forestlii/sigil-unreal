// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Attributes/AS_Combat.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "GGA_GameplayAttributesHelper.h"
#include "GGA_AttributeSystemComponent.h"


namespace AS_Combat
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage, TEXT("GGF.Attribute.CombatSet.Damage"), "The damage that will apply to target")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(DamageNegation, TEXT("GGF.Attribute.CombatSet.DamageNegation"), "The damage reduction(percentage) for incoming health damage")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(GuardDamageNegation, TEXT("GGF.Attribute.CombatSet.GuardDamageNegation"), "The damage reduction(percentage) for incoming health damage while guarding")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(StaminaDamage, TEXT("GGF.Attribute.CombatSet.StaminaDamage"), "The stamina damage that will apply to target")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(StaminaDamageNegation, TEXT("GGF.Attribute.CombatSet.StaminaDamageNegation"), "The damage reduction(percentage) for incoming stamina damage")


}

UAS_Combat::UAS_Combat()
{

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Combat::Damage,GetDamageAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Combat::DamageNegation,GetDamageNegationAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Combat::GuardDamageNegation,GetGuardDamageNegationAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Combat::StaminaDamage,GetStaminaDamageAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Combat::StaminaDamageNegation,GetStaminaDamageNegationAttribute());


}

void UAS_Combat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Damage, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, DamageNegation, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, GuardDamageNegation, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, StaminaDamage, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, StaminaDamageNegation, COND_None, REPNOTIFY_Always);
    
}


void UAS_Combat::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    

    

    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void UAS_Combat::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    

    

    

    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void UAS_Combat::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    

    

    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void UAS_Combat::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute UAS_Combat::Bp_GetDamageAttribute()
{
    return ThisClass::GetDamageAttribute();
}

float UAS_Combat::Bp_GetDamage() const
{
    return GetDamage();
}

void UAS_Combat::Bp_SetDamage(float NewValue)
{
    SetDamage(NewValue);
}

void UAS_Combat::Bp_InitDamage(float NewValue)
{
    InitDamage(NewValue);
}

void UAS_Combat::OnRep_Damage(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Damage, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetDamageAttribute(),GetDamage(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Combat::Bp_GetDamageNegationAttribute()
{
    return ThisClass::GetDamageNegationAttribute();
}

float UAS_Combat::Bp_GetDamageNegation() const
{
    return GetDamageNegation();
}

void UAS_Combat::Bp_SetDamageNegation(float NewValue)
{
    SetDamageNegation(NewValue);
}

void UAS_Combat::Bp_InitDamageNegation(float NewValue)
{
    InitDamageNegation(NewValue);
}

void UAS_Combat::OnRep_DamageNegation(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, DamageNegation, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetDamageNegationAttribute(),GetDamageNegation(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Combat::Bp_GetGuardDamageNegationAttribute()
{
    return ThisClass::GetGuardDamageNegationAttribute();
}

float UAS_Combat::Bp_GetGuardDamageNegation() const
{
    return GetGuardDamageNegation();
}

void UAS_Combat::Bp_SetGuardDamageNegation(float NewValue)
{
    SetGuardDamageNegation(NewValue);
}

void UAS_Combat::Bp_InitGuardDamageNegation(float NewValue)
{
    InitGuardDamageNegation(NewValue);
}

void UAS_Combat::OnRep_GuardDamageNegation(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, GuardDamageNegation, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetGuardDamageNegationAttribute(),GetGuardDamageNegation(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Combat::Bp_GetStaminaDamageAttribute()
{
    return ThisClass::GetStaminaDamageAttribute();
}

float UAS_Combat::Bp_GetStaminaDamage() const
{
    return GetStaminaDamage();
}

void UAS_Combat::Bp_SetStaminaDamage(float NewValue)
{
    SetStaminaDamage(NewValue);
}

void UAS_Combat::Bp_InitStaminaDamage(float NewValue)
{
    InitStaminaDamage(NewValue);
}

void UAS_Combat::OnRep_StaminaDamage(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, StaminaDamage, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetStaminaDamageAttribute(),GetStaminaDamage(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Combat::Bp_GetStaminaDamageNegationAttribute()
{
    return ThisClass::GetStaminaDamageNegationAttribute();
}

float UAS_Combat::Bp_GetStaminaDamageNegation() const
{
    return GetStaminaDamageNegation();
}

void UAS_Combat::Bp_SetStaminaDamageNegation(float NewValue)
{
    SetStaminaDamageNegation(NewValue);
}

void UAS_Combat::Bp_InitStaminaDamageNegation(float NewValue)
{
    InitStaminaDamageNegation(NewValue);
}

void UAS_Combat::OnRep_StaminaDamageNegation(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, StaminaDamageNegation, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetStaminaDamageNegationAttribute(),GetStaminaDamageNegation(),OldValue.GetCurrentValue());
        }
    }    
}



