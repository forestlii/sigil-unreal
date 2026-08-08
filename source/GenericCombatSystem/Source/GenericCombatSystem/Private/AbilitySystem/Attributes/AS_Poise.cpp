// Copyright 2025 RedMoonGames All Rights Reserved.

#include "GenericCombatSystem/Public/AbilitySystem/Attributes/AS_Poise.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "GGA_GameplayAttributesHelper.h"
#include "GGA_AttributeSystemComponent.h"


namespace AS_Poise
{

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Poise, TEXT("GGF.Attribute.PoiseSet.Poise"), "Current Poise value of an actor.(actor的当前抗打击值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(MaxPoise, TEXT("GGF.Attribute.PoiseSet.MaxPoise"), "Max Poise value of an actor.(actor的最大抗打击值)")

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(PoiseRecover, TEXT("GGF.Attribute.PoiseSet.PoiseRecover"), "How many Poise to recover per second.(每秒恢复抗打击值)")


}

UAS_Poise::UAS_Poise()
{

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Poise::Poise,GetPoiseAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Poise::MaxPoise,GetMaxPoiseAttribute());

    UGGA_GameplayAttributesHelper::RegisterTagToAttribute(AS_Poise::PoiseRecover,GetPoiseRecoverAttribute());


}

void UAS_Poise::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Poise, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxPoise, COND_None, REPNOTIFY_Always);
    
    DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, PoiseRecover, COND_None, REPNOTIFY_Always);
    
}


void UAS_Poise::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    
    if (Attribute == GetPoiseAttribute())
    {
        NewValue = FMath::Clamp(NewValue,0,GetMaxPoise());
    }
    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePreAttributeChange(this,Attribute,NewValue);
        }
    }    
}

void UAS_Poise::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);


    
    if (Attribute == GetMaxPoiseAttribute())
    {
        AdjustAttributeForMaxChange(Poise, OldValue, NewValue, GetPoiseAttribute());
    }
    

    

    


	if (AActor* Actor = GetOwningActor())
	{
		if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
		{
			ASS->ReceivePostAttributeChange(this, Attribute, OldValue, NewValue);
		}
	}
}

void UAS_Poise::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    
    if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
    {
        SetPoise(FMath::Clamp(GetPoise(),0,GetMaxPoise()));
    }
    

    

    


    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceivePostGameplayEffectExecute(this,Data);
        }
    }
}

void UAS_Poise::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
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



FGameplayAttribute UAS_Poise::Bp_GetPoiseAttribute()
{
    return ThisClass::GetPoiseAttribute();
}

float UAS_Poise::Bp_GetPoise() const
{
    return GetPoise();
}

void UAS_Poise::Bp_SetPoise(float NewValue)
{
    SetPoise(NewValue);
}

void UAS_Poise::Bp_InitPoise(float NewValue)
{
    InitPoise(NewValue);
}

void UAS_Poise::OnRep_Poise(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Poise, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetPoiseAttribute(),GetPoise(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Poise::Bp_GetMaxPoiseAttribute()
{
    return ThisClass::GetMaxPoiseAttribute();
}

float UAS_Poise::Bp_GetMaxPoise() const
{
    return GetMaxPoise();
}

void UAS_Poise::Bp_SetMaxPoise(float NewValue)
{
    SetMaxPoise(NewValue);
}

void UAS_Poise::Bp_InitMaxPoise(float NewValue)
{
    InitMaxPoise(NewValue);
}

void UAS_Poise::OnRep_MaxPoise(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxPoise, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetMaxPoiseAttribute(),GetMaxPoise(),OldValue.GetCurrentValue());
        }
    }    
}



FGameplayAttribute UAS_Poise::Bp_GetPoiseRecoverAttribute()
{
    return ThisClass::GetPoiseRecoverAttribute();
}

float UAS_Poise::Bp_GetPoiseRecover() const
{
    return GetPoiseRecover();
}

void UAS_Poise::Bp_SetPoiseRecover(float NewValue)
{
    SetPoiseRecover(NewValue);
}

void UAS_Poise::Bp_InitPoiseRecover(float NewValue)
{
    InitPoiseRecover(NewValue);
}

void UAS_Poise::OnRep_PoiseRecover(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, PoiseRecover, OldValue);
    if (AActor* Actor = GetOwningActor())
    {
        if (UGGA_AttributeSystemComponent* ASS = Actor->FindComponentByClass<UGGA_AttributeSystemComponent>())
        {
            ASS->ReceiveAttributeChange(this,GetPoiseRecoverAttribute(),GetPoiseRecover(),OldValue.GetCurrentValue());
        }
    }    
}



