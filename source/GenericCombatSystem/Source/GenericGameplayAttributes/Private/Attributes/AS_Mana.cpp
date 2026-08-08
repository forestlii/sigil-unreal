// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Attributes/AS_Mana.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "GGA_GameplayAttributesHelper.h"
#include "GGA_AttributeSystemComponent.h"


namespace AS_Mana
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mana, TEXT("GGF.Attribute.ManaSet.Mana"), "Current mana of an actor.(actor的当前魔法值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxMana, TEXT("GGF.Attribute.ManaSet.MaxMana"), "Max mana value of an actor.(actor的最大魔法值)")


}

UAS_Mana::UAS_Mana()
{

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Mana::Mana,GetManaAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Mana::MaxMana,GetMaxManaAttribute());


}

void UAS_Mana::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Mana, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxMana, COND_None, REPNOTIFY_Always);
    
}


void UAS_Mana::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxMana());
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void UAS_Mana::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxManaAttribute())
    {
        AdjustAttributeForMaxChange(Mana, OldValue, NewValue, GetManaAttribute());
    }
    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void UAS_Mana::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(),0,GetMaxMana()));
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void UAS_Mana::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute UAS_Mana::Bp_GetManaAttribute()
{
    return ThisClass::GetManaAttribute();
}

float UAS_Mana::Bp_GetMana() const
{
    return GetMana();
}

void UAS_Mana::Bp_SetMana(float NewValue)
{
    SetMana(NewValue);
}

void UAS_Mana::Bp_InitMana(float NewValue)
{
    InitMana(NewValue);
}

void UAS_Mana::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Mana, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetManaAttribute(),GetMana(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Mana::Bp_GetMaxManaAttribute()
{
    return ThisClass::GetMaxManaAttribute();
}

float UAS_Mana::Bp_GetMaxMana() const
{
    return GetMaxMana();
}

void UAS_Mana::Bp_SetMaxMana(float NewValue)
{
    SetMaxMana(NewValue);
}

void UAS_Mana::Bp_InitMaxMana(float NewValue)
{
    InitMaxMana(NewValue);
}

void UAS_Mana::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxMana, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxManaAttribute(),GetMaxMana(),OldValue.GetCurrentValue());
        }
    }    
}



