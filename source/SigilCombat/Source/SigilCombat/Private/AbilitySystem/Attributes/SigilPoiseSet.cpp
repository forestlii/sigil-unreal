// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilCombat/Public/AbilitySystem/Attributes/SigilPoiseSet.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "SigilGameplayAttributesHelper.h"
#include "SigilAttributeSystemComponent.h"


namespace SigilPoiseSet
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Poise, TEXT("Sigil.Attribute.PoiseSet.Poise"), "Current Poise value of an actor.(actor的当前抗打击值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxPoise, TEXT("Sigil.Attribute.PoiseSet.MaxPoise"), "Max Poise value of an actor.(actor的最大抗打击值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(PoiseRecover, TEXT("Sigil.Attribute.PoiseSet.PoiseRecover"), "How many Poise to recover per second.(每秒恢复抗打击值)")


}

USigilPoiseSet::USigilPoiseSet()
{

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilPoiseSet::Poise,GetPoiseAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilPoiseSet::MaxPoise,GetMaxPoiseAttribute());

    USigilGameplayAttributesHelper::RegisterTagToAttribute(SigilPoiseSet::PoiseRecover,GetPoiseRecoverAttribute());


}

void USigilPoiseSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Poise, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxPoise, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, PoiseRecover, COND_None, REPNOTIFY_Always);
    
}


void USigilPoiseSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetPoiseAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxPoise());
    }
    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void USigilPoiseSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxPoiseAttribute())
    {
        AdjustAttributeForMaxChange(Poise, OldValue, NewValue, GetPoiseAttribute());
    }
    

    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void USigilPoiseSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
    {
        SetPoise(FMath::Clamp(GetPoise(),0,GetMaxPoise()));
    }
    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void USigilPoiseSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute USigilPoiseSet::Bp_GetPoiseAttribute()
{
    return ThisClass::GetPoiseAttribute();
}

float USigilPoiseSet::Bp_GetPoise() const
{
    return GetPoise();
}

void USigilPoiseSet::Bp_SetPoise(float NewValue)
{
    SetPoise(NewValue);
}

void USigilPoiseSet::Bp_InitPoise(float NewValue)
{
    InitPoise(NewValue);
}

void USigilPoiseSet::OnRep_Poise(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Poise, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetPoiseAttribute(),GetPoise(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilPoiseSet::Bp_GetMaxPoiseAttribute()
{
    return ThisClass::GetMaxPoiseAttribute();
}

float USigilPoiseSet::Bp_GetMaxPoise() const
{
    return GetMaxPoise();
}

void USigilPoiseSet::Bp_SetMaxPoise(float NewValue)
{
    SetMaxPoise(NewValue);
}

void USigilPoiseSet::Bp_InitMaxPoise(float NewValue)
{
    InitMaxPoise(NewValue);
}

void USigilPoiseSet::OnRep_MaxPoise(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxPoise, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxPoiseAttribute(),GetMaxPoise(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute USigilPoiseSet::Bp_GetPoiseRecoverAttribute()
{
    return ThisClass::GetPoiseRecoverAttribute();
}

float USigilPoiseSet::Bp_GetPoiseRecover() const
{
    return GetPoiseRecover();
}

void USigilPoiseSet::Bp_SetPoiseRecover(float NewValue)
{
    SetPoiseRecover(NewValue);
}

void USigilPoiseSet::Bp_InitPoiseRecover(float NewValue)
{
    InitPoiseRecover(NewValue);
}

void USigilPoiseSet::OnRep_PoiseRecover(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, PoiseRecover, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (USigilAttributeSystemComponent* ASS = Actor->FindComponentByClass<USigilAttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetPoiseRecoverAttribute(),GetPoiseRecover(),OldValue.GetCurrentValue());
        }
    }    
}



