// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilAttributeSystemComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogSigilAttributeSystem, Log, All);

// Sets default values for this component's properties
USigilAttributeSystemComponent::USigilAttributeSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


bool USigilAttributeSystemComponent::ReceivePreGameplayEffectExecute(UAttributeSet* AttributeSet, FGameplayEffectModCallbackData& Data)
{
	return true;
}

void USigilAttributeSystemComponent::ReceivePostGameplayEffectExecute(UAttributeSet* AttributeSet, const FGameplayEffectModCallbackData& Data)
{
	if (!AttributeSet)
	{
		UE_LOG(LogSigilAttributeSystem, Error, TEXT("Owner AttributeSet isn't valid"));
		return;
	}

	FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();

	FSigilGameplayEffectModCallbackData Payload;
	Payload.AttributeSet = AttributeSet;
	Payload.EvaluatedData = Data.EvaluatedData;

	for (const FGameplayEffectModifiedAttribute& ModifiedAttribute : Data.EffectSpec.ModifiedAttributes)
	{
		FSigilModifiedAttribute ModAttribute;
		ModAttribute.Attribute = ModifiedAttribute.Attribute;
		ModAttribute.TotalMagnitude = ModifiedAttribute.TotalMagnitude;
		Payload.ModifiedAttributes.Add(ModAttribute);
	}

	Payload.SetByCallerNameMagnitudes = Data.EffectSpec.SetByCallerNameMagnitudes;
	Payload.SetByCallerTagMagnitudes = Data.EffectSpec.SetByCallerTagMagnitudes;

	Payload.ContextHandle = ContextHandle;
	Payload.InstigatorActor = Data.EffectSpec.GetContext().GetInstigator();

	Payload.TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
	Payload.TargetAsc = &Data.Target;

	Payload.AggregatedSourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	Payload.AggregatedTargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
	OnPostGameplayEffectExecute.Broadcast(Payload);
	HandlePostGameplayEffectExecute(Payload);
}

void USigilAttributeSystemComponent::ReceivePreAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float& NewValue)
{
	NewValue = HandlePreAttributeChange(AttributeSet, Attribute, NewValue);
}

void USigilAttributeSystemComponent::ReceivePostAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	HandlePostAttributeChange(AttributeSet, Attribute, OldValue, NewValue);
	OnPostAttributeChange.Broadcast(AttributeSet, Attribute, OldValue, NewValue);
}

void USigilAttributeSystemComponent::ReceiveAttributeChange(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, const float& NewValue, const float& OldValue)
{
	HandleAttributeChange(AttributeSet, Attribute, NewValue, OldValue);
	OnAttributeChanged.Broadcast(AttributeSet, Attribute, NewValue, OldValue);
}

float USigilAttributeSystemComponent::HandlePreAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float NewValue)
{
	return NewValue;
}

void USigilAttributeSystemComponent::HandlePostAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
}

void USigilAttributeSystemComponent::HandlePostGameplayEffectExecute_Implementation(const FSigilGameplayEffectModCallbackData& Payload)
{
}

void USigilAttributeSystemComponent::HandleAttributeChange_Implementation(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, const float& NewValue, const float& OldValue)
{
}

// Called when the game starts
void USigilAttributeSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}
