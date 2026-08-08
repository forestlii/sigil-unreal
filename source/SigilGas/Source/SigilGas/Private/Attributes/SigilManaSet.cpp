// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Attributes/SigilManaSet.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "SigilGameplayAttributesHelper.h"
#include "SigilAttributeSystemComponent.h"


namespace SigilManaSet
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mana, TEXT("Sigil.Attribute.ManaSet.Mana"), "Current mana of an actor.(actor的当前魔法值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxMana, TEXT("Sigil.Attribute.ManaSet.MaxMana"), "Max mana value of an actor.(actor的最大魔法值)")


}

USigilManaSet::USigilManaSet()
{

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilManaSet::Mana,GetManaAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilManaSet::MaxMana,GetMaxManaAttribute());


}

void USigilManaSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Mana, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxMana, COND_None, REPNOTIFY_Always);
    
}


void USigilManaSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxMana());
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void USigilManaSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxManaAttribute())
    {
        AdjustAttributeForMaxChange(Mana, OldValue, NewValue, GetManaAttribute());
    }
    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void USigilManaSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(),0,GetMaxMana()));
    }
    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void USigilManaSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute USigilManaSet::Bp_GetManaAttribute()
{
    return ThisClass::GetManaAttribute();
}

float USigilManaSet::Bp_GetMana() const
{
    return GetMana();
}

void USigilManaSet::Bp_SetMana(float NewValue)
{
    SetMana(NewValue);
}

void USigilManaSet::Bp_InitMana(float NewValue)
{
    InitMana(NewValue);
}

void USigilManaSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Mana, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetManaAttribute(),GetMana(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilManaSet::Bp_GetMaxManaAttribute()
{
    return ThisClass::GetMaxManaAttribute();
}

float USigilManaSet::Bp_GetMaxMana() const
{
    return GetMaxMana();
}

void USigilManaSet::Bp_SetMaxMana(float NewValue)
{
    SetMaxMana(NewValue);
}

void USigilManaSet::Bp_InitMaxMana(float NewValue)
{
    InitMaxMana(NewValue);
}

void USigilManaSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxMana, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxManaAttribute(),GetMaxMana(),OldValue.GetCurrentValue());
        }
    }    
}



