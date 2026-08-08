// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utilities/SigilGameplayEffectCalculationFunctionLibrary.h"

#include "AbilitySystemComponent.h"

const FGameplayEffectSpec& USigilGameplayEffectCalculationFunctionLibrary::GetOwningSpec(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetOwningSpec();
}

FGameplayEffectContextHandle USigilGameplayEffectCalculationFunctionLibrary::GetEffectContext(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetOwningSpec().GetEffectContext();
}

float USigilGameplayEffectCalculationFunctionLibrary::GetSetByCallerMagnitudeByTag(const FGameplayEffectCustomExecutionParameters& InParams, const FGameplayTag& Tag, bool WarnIfNotFound,
                                                                                  float DefaultIfNotFound)
{
	return InParams.GetOwningSpec().GetSetByCallerMagnitude(Tag, WarnIfNotFound, DefaultIfNotFound);
}

float USigilGameplayEffectCalculationFunctionLibrary::GetSetByCallerMagnitudeByName(const FGameplayEffectCustomExecutionParameters& InParams, const FName& MagnitudeName, bool WarnIfNotFound,
                                                                                   float DefaultIfNotFound)
{
	return InParams.GetOwningSpec().GetSetByCallerMagnitude(MagnitudeName, WarnIfNotFound, DefaultIfNotFound);
}


FGameplayTagContainer USigilGameplayEffectCalculationFunctionLibrary::GetSourceAggregatedTags(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return *InParams.GetOwningSpec().CapturedSourceTags.GetAggregatedTags();
}

FGameplayTagContainer USigilGameplayEffectCalculationFunctionLibrary::GetTargetAggregatedTags(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return *InParams.GetOwningSpec().CapturedTargetTags.GetAggregatedTags();
}

UAbilitySystemComponent* USigilGameplayEffectCalculationFunctionLibrary::GetTargetASC(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetTargetAbilitySystemComponent();
}

AActor* USigilGameplayEffectCalculationFunctionLibrary::GetTargetActor(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetTargetAbilitySystemComponent()->GetAvatarActor();
}

UAbilitySystemComponent* USigilGameplayEffectCalculationFunctionLibrary::GetSourceASC(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetSourceAbilitySystemComponent();
}

AActor* USigilGameplayEffectCalculationFunctionLibrary::GetSourceActor(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetSourceAbilitySystemComponent()->GetAvatarActor();
}

bool USigilGameplayEffectCalculationFunctionLibrary::AttemptCalculateCapturedAttributeMagnitude(const FGameplayEffectCustomExecutionParameters& InParams,
                                                                                               TArray<FGameplayEffectAttributeCaptureDefinition> InAttributeCaptureDefinitions,
                                                                                               FGameplayAttribute InAttribute, float& OutMagnitude)
{
	FAggregatorEvaluateParameters EvaluationParams;
	const FGameplayEffectSpec& EffectSpec = InParams.GetOwningSpec();
	EvaluationParams.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParams.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	for (const FGameplayEffectAttributeCaptureDefinition& AttributeCaptureDefinition : InAttributeCaptureDefinitions)
	{
		if (AttributeCaptureDefinition.AttributeToCapture == InAttribute)
		{
			return InParams.AttemptCalculateCapturedAttributeMagnitude(AttributeCaptureDefinition, EvaluationParams, OutMagnitude);
		}
	}
	return false;
}

bool USigilGameplayEffectCalculationFunctionLibrary::AttemptCalculateCapturedAttributeMagnitudeExt(const FGameplayEffectCustomExecutionParameters& InParams, const FGameplayTagContainer& SourceTags,
                                                                                                  const FGameplayTagContainer& TargetTags,
                                                                                                  TArray<FGameplayEffectAttributeCaptureDefinition> InAttributeCaptureDefinitions,
                                                                                                  FGameplayAttribute InAttribute, float& OutMagnitude)
{
	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = &SourceTags;
	EvaluationParams.TargetTags = &TargetTags;

	for (const FGameplayEffectAttributeCaptureDefinition& AttributeCaptureDefinition : InAttributeCaptureDefinitions)
	{
		if (AttributeCaptureDefinition.AttributeToCapture == InAttribute)
		{
			return InParams.AttemptCalculateCapturedAttributeMagnitude(AttributeCaptureDefinition, EvaluationParams, OutMagnitude);
		}
	}
	return false;
}

bool USigilGameplayEffectCalculationFunctionLibrary::AttemptCalculateCapturedAttributeMagnitudeWithBase(const FGameplayEffectCustomExecutionParameters& InParams,
                                                                                                       TArray<FGameplayEffectAttributeCaptureDefinition> InAttributeCaptureDefinitions,
                                                                                                       FGameplayAttribute InAttribute, float InBaseValue, float& OutMagnitude)
{
	FAggregatorEvaluateParameters EvaluationParams;
	const FGameplayEffectSpec& EffectSpec = InParams.GetOwningSpec();
	EvaluationParams.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParams.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	for (const FGameplayEffectAttributeCaptureDefinition& AttributeCaptureDefinition : InAttributeCaptureDefinitions)
	{
		if (AttributeCaptureDefinition.AttributeToCapture == InAttribute)
		{
			return InParams.AttemptCalculateCapturedAttributeMagnitudeWithBase(AttributeCaptureDefinition, EvaluationParams, InBaseValue, OutMagnitude);
		}
	}
	return false;
}

FGameplayEffectCustomExecutionOutput USigilGameplayEffectCalculationFunctionLibrary::AddOutputModifier(FGameplayEffectCustomExecutionOutput& InExecutionOutput, FGameplayAttribute InAttribute,
                                                                                                      EGameplayModOp::Type InModifierOp, float InMagnitude)
{
	if (InAttribute.IsValid())
	{
		FGameplayModifierEvaluatedData Data;
		Data.Attribute = InAttribute;
		Data.ModifierOp = InModifierOp;
		Data.Magnitude = InMagnitude;
		InExecutionOutput.AddOutputModifier(Data);
	}
	return InExecutionOutput;
}


void USigilGameplayEffectCalculationFunctionLibrary::MarkConditionalGameplayEffectsToTrigger(FGameplayEffectCustomExecutionOutput& InExecutionOutput)
{
	InExecutionOutput.MarkConditionalGameplayEffectsToTrigger();
}

void USigilGameplayEffectCalculationFunctionLibrary::MarkGameplayCuesHandledManually(FGameplayEffectCustomExecutionOutput& InExecutionOutput)
{
	InExecutionOutput.MarkGameplayCuesHandledManually();
}

void USigilGameplayEffectCalculationFunctionLibrary::MarkStackCountHandledManually(FGameplayEffectCustomExecutionOutput& InExecutionOutput)
{
	InExecutionOutput.MarkStackCountHandledManually();
}

FGameplayEffectContextHandle USigilGameplayEffectCalculationFunctionLibrary::GetEffectContextFromSpec(const FGameplayEffectSpec& EffectSpec)
{
	return EffectSpec.GetEffectContext();
}

void USigilGameplayEffectCalculationFunctionLibrary::AddAssetTagForPreMod(const FGameplayEffectCustomExecutionParameters& InParams, FGameplayTag NewGameplayTag)
{
	if (FGameplayEffectSpec* Spec = InParams.GetOwningSpecForPreExecuteMod())
	{
		Spec->AddDynamicAssetTag(NewGameplayTag);
	}
}

void USigilGameplayEffectCalculationFunctionLibrary::AddAssetTagsForPreMod(const FGameplayEffectCustomExecutionParameters& InParams, FGameplayTagContainer NewGameplayTags)
{
	if (FGameplayEffectSpec* Spec = InParams.GetOwningSpecForPreExecuteMod())
	{
		Spec->AppendDynamicAssetTags(NewGameplayTags);
	}
}

