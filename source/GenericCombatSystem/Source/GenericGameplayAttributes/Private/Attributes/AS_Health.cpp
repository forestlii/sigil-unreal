// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Attributes/AS_Health.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "GGA_GameplayAttributesHelper.h"
#include "GGA_AttributeSystemComponent.h"


namespace AS_Health
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Health, TEXT("GGF.Attribute.HealthSet.Health"), "Current health of an actor.(actor的当前生命值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxHealth, TEXT("GGF.Attribute.HealthSet.MaxHealth"), "Max health value of an actor.(actor的最大生命值)")


    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingHealing, TEXT("GGF.Attribute.HealthSet.IncomingHealing"), "Incoming healing. This is mapped directly to +Health.(即将到来的恢复值，映射为+Health)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(IncomingDamage, TEXT("GGF.Attribute.HealthSet.IncomingDamage"), "Incoming damage. This is mapped directly to -Health(即将到来的伤害值，映射为-Health)")

}

UAS_Health::UAS_Health()
{

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Health::Health,GetHealthAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Health::MaxHealth,GetMaxHealthAttribute());


    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Health::IncomingHealing,GetIncomingHealingAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Health::IncomingDamage,GetIncomingDamageAttribute());

}

void UAS_Health::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Health, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxHealth, COND_None, REPNOTIFY_Always);
    
}


void UAS_Health::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxHealth());
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void UAS_Health::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxHealthAttribute())
    {
        AdjustAttributeForMaxChange(Health, OldValue, NewValue, GetHealthAttribute());
    }
    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void UAS_Health::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(),0,GetMaxHealth()));
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void UAS_Health::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute UAS_Health::Bp_GetHealthAttribute()
{
    return ThisClass::GetHealthAttribute();
}

float UAS_Health::Bp_GetHealth() const
{
    return GetHealth();
}

void UAS_Health::Bp_SetHealth(float NewValue)
{
    SetHealth(NewValue);
}

void UAS_Health::Bp_InitHealth(float NewValue)
{
    InitHealth(NewValue);
}

void UAS_Health::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetHealthAttribute(),GetHealth(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Health::Bp_GetMaxHealthAttribute()
{
    return ThisClass::GetMaxHealthAttribute();
}

float UAS_Health::Bp_GetMaxHealth() const
{
    return GetMaxHealth();
}

void UAS_Health::Bp_SetMaxHealth(float NewValue)
{
    SetMaxHealth(NewValue);
}

void UAS_Health::Bp_InitMaxHealth(float NewValue)
{
    InitMaxHealth(NewValue);
}

void UAS_Health::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxHealthAttribute(),GetMaxHealth(),OldValue.GetCurrentValue());
        }
    }    
}





FGameplayAttribute UAS_Health::Bp_GetIncomingHealingAttribute()
{
    return ThisClass::GetIncomingHealingAttribute();
}

float UAS_Health::Bp_GetIncomingHealing() const
{
    return GetIncomingHealing();
}

void UAS_Health::Bp_SetIncomingHealing(float NewValue)
{
    SetIncomingHealing(NewValue);
}



FGameplayAttribute UAS_Health::Bp_GetIncomingDamageAttribute()
{
    return ThisClass::GetIncomingDamageAttribute();
}

float UAS_Health::Bp_GetIncomingDamage() const
{
    return GetIncomingDamage();
}

void UAS_Health::Bp_SetIncomingDamage(float NewValue)
{
    SetIncomingDamage(NewValue);
}

