// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Attributes/SigilHealthSet.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "SigilGameplayAttributesHelper.h"
#include "SigilAttributeSystemComponent.h"


namespace SigilHealthSet
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Health, TEXT("Sigil.Attribute.HealthSet.Health"), "Current health of an actor.(actor的当前生命值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxHealth, TEXT("Sigil.Attribute.HealthSet.MaxHealth"), "Max health value of an actor.(actor的最大生命值)")


    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingHealing, TEXT("Sigil.Attribute.HealthSet.IncomingHealing"), "Incoming healing. This is mapped directly to +Health.(即将到来的恢复值，映射为+Health)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingDamage, TEXT("Sigil.Attribute.HealthSet.IncomingDamage"), "Incoming damage. This is mapped directly to -Health(即将到来的伤害值，映射为-Health)")

}

USigilHealthSet::USigilHealthSet()
{

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilHealthSet::Health,GetHealthAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilHealthSet::MaxHealth,GetMaxHealthAttribute());


    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilHealthSet::IncomingHealing,GetIncomingHealingAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilHealthSet::IncomingDamage,GetIncomingDamageAttribute());

}

void USigilHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Health, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxHealth, COND_None, REPNOTIFY_Always);
    
}


void USigilHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxHealth());
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void USigilHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxHealthAttribute())
    {
        AdjustAttributeForMaxChange(Health, OldValue, NewValue, GetHealthAttribute());
    }
    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void USigilHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(),0,GetMaxHealth()));
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void USigilHealthSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute USigilHealthSet::Bp_GetHealthAttribute()
{
    return ThisClass::GetHealthAttribute();
}

float USigilHealthSet::Bp_GetHealth() const
{
    return GetHealth();
}

void USigilHealthSet::Bp_SetHealth(float NewValue)
{
    SetHealth(NewValue);
}

void USigilHealthSet::Bp_InitHealth(float NewValue)
{
    InitHealth(NewValue);
}

void USigilHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetHealthAttribute(),GetHealth(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilHealthSet::Bp_GetMaxHealthAttribute()
{
    return ThisClass::GetMaxHealthAttribute();
}

float USigilHealthSet::Bp_GetMaxHealth() const
{
    return GetMaxHealth();
}

void USigilHealthSet::Bp_SetMaxHealth(float NewValue)
{
    SetMaxHealth(NewValue);
}

void USigilHealthSet::Bp_InitMaxHealth(float NewValue)
{
    InitMaxHealth(NewValue);
}

void USigilHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxHealthAttribute(),GetMaxHealth(),OldValue.GetCurrentValue());
        }
    }    
}





FGameplayAttribute USigilHealthSet::Bp_GetIncomingHealingAttribute()
{
    return ThisClass::GetIncomingHealingAttribute();
}

float USigilHealthSet::Bp_GetIncomingHealing() const
{
    return GetIncomingHealing();
}

void USigilHealthSet::Bp_SetIncomingHealing(float NewValue)
{
    SetIncomingHealing(NewValue);
}



FGameplayAttribute USigilHealthSet::Bp_GetIncomingDamageAttribute()
{
    return ThisClass::GetIncomingDamageAttribute();
}

float USigilHealthSet::Bp_GetIncomingDamage() const
{
    return GetIncomingDamage();
}

void USigilHealthSet::Bp_SetIncomingDamage(float NewValue)
{
    SetIncomingDamage(NewValue);
}

