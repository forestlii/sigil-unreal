// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GGA_AttributeSystemComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogGGA_AttributeSystem, Log, All);

// Sets default values for this component's properties
UGGA_AttributeSystemComponent::UGGA_AttributeSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


bool UGGA_AttributeSystemComponent::ReceivePreGameplayEffectExecute(UAttributeSet* AttributeSet, FGameplayEffectModCallbackData& Data)
{
	return true;
}

void UGGA_AttributeSystemComponent::ReceivePostGameplayEffectExecute(UAttributeSet* AttributeSet, const FGameplayEffectModCallbackData& Data)
{
	if (!AttributeSet)
	{
		UE_LOG(LogGGA_AttributeSystem, Error, TEXT("Owner AttributeSet isn't valid"));
		return;
	}

	FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();

	FGGA_GameplayEffectModCallbackData Payload;
	Payload.AttributeSet = AttributeSet;
	Payload.EvaluatedData = Data.EvaluatedData;

	for (const FGameplayEffectModifiedAttribute& ModifiedAttribute : Data.EffectSpec.ModifiedAttributes)
	{
		FGGA_ModifiedAttribute ModAttribute;
		ModAttribute.Attribute = ModifiedAttribute.Attribute;
		ModAttribute.TotalMagnitude = ModifiedAttribute.TotalMagnitude;
		Payload.ModifiedAttributes.Add(ModAttribute);
	}

	Payload.SetByCallerNameMagnitudes = Data.EffectSpec.SetByCallerNameMagnitudes;
	Payload.SetByCallerTagMagnitudes = Data.EffectSpec.SetByCallerTagMagnitudes;

	Payload.ContextHandle = ContextHandle;
	Payload.InstigatorActor = Data.EffectSpec.GetContext().GetInstigator();

	Payload.TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();;
	Payload.TargetAsc = Data.Target;

	Payload.AggregatedSourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	Payload.AggregatedTargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
	OnPostGameplayEffectExecute.Broadcast(Payload);
	HandlePostGameplayEffectExecute(Payload);
}

void UGGA_AttributeSystemComponent::ReceivePreAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float& NewValue)
{
	NewValue = HandlePreAttributeChange(AttributeSet, Attribute, NewValue);
}

void UGGA_AttributeSystemComponent::ReceivePostAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	HandlePostAttributeChange(AttributeSet, Attribute, OldValue, NewValue);
	OnPostAttributeChange.Broadcast(AttributeSet, Attribute, OldValue, NewValue);
}

void UGGA_AttributeSystemComponent::ReceiveAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, const float& NewValue, const float& OldValue)
{
	HandleAttributeChange(AttributeSet, Attribute, NewValue, OldValue);
	OnAttributeChanged.Broadcast(AttributeSet, Attribute, NewValue, OldValue);
}

float UGGA_AttributeSystemComponent::HandlePreAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float NewValue)
{
	return NewValue;
}

void UGGA_AttributeSystemComponent::HandlePostAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
}

void UGGA_AttributeSystemComponent::HandlePostGameplayEffectExecute_Implementation(const FGGA_GameplayEffectModCallbackData& Payload)
{
}

void UGGA_AttributeSystemComponent::HandleAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, const float& NewValue, const float& OldValue)
{
}

// Called when the game starts
void UGGA_AttributeSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}
