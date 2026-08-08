// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utilities/GGA_GameplayEffectCalculationFunctionLibrary.h"

#include "AbilitySystemComponent.h"

const FGameplayEffectSpec& UGGA_GameplayEffectCalculationFunctionLibrary::GetOwningSpec(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetOwningSpec();
}

FGameplayEffectContextHandle UGGA_GameplayEffectCalculationFunctionLibrary::GetEffectContext(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetOwningSpec().GetEffectContext();
}

float UGGA_GameplayEffectCalculationFunctionLibrary::GetSetByCallerMagnitudeByTag(const FGameplayEffectCustomExecutionParameters& InParams, const FGameplayTag& Tag, bool WarnIfNotFound,
                                                                                  float DefaultIfNotFound)
{
	return InParams.GetOwningSpec().GetSetByCallerMagnitude(Tag, WarnIfNotFound, DefaultIfNotFound);
}

float UGGA_GameplayEffectCalculationFunctionLibrary::GetSetByCallerMagnitudeByName(const FGameplayEffectCustomExecutionParameters& InParams, const FName& MagnitudeName, bool WarnIfNotFound,
                                                                                   float DefaultIfNotFound)
{
	return InParams.GetOwningSpec().GetSetByCallerMagnitude(MagnitudeName, WarnIfNotFound, DefaultIfNotFound);
}


FGameplayTagContainer UGGA_GameplayEffectCalculationFunctionLibrary::GetSourceAggregatedTags(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return *InParams.GetOwningSpec().CapturedSourceTags.GetAggregatedTags();
}

FGameplayTagContainer UGGA_GameplayEffectCalculationFunctionLibrary::GetTargetAggregatedTags(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return *InParams.GetOwningSpec().CapturedTargetTags.GetAggregatedTags();
}

UAbilitySystemComponent* UGGA_GameplayEffectCalculationFunctionLibrary::GetTargetASC(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetTargetAbilitySystemComponent();
}

AActor* UGGA_GameplayEffectCalculationFunctionLibrary::GetTargetActor(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetTargetAbilitySystemComponent()->GetAvatarActor();
}

UAbilitySystemComponent* UGGA_GameplayEffectCalculationFunctionLibrary::GetSourceASC(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetSourceAbilitySystemComponent();
}

AActor* UGGA_GameplayEffectCalculationFunctionLibrary::GetSourceActor(const FGameplayEffectCustomExecutionParameters& InParams)
{
	return InParams.GetSourceAbilitySystemComponent()->GetAvatarActor();
}

bool UGGA_GameplayEffectCalculationFunctionLibrary::AttemptCalculateCapturedAttributeMagnitude(const FGameplayEffectCustomExecutionParameters& InParams,
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

bool UGGA_GameplayEffectCalculationFunctionLibrary::AttemptCalculateCapturedAttributeMagnitudeExt(const FGameplayEffectCustomExecutionParameters& InParams, const FGameplayTagContainer& SourceTags,
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

bool UGGA_GameplayEffectCalculationFunctionLibrary::AttemptCalculateCapturedAttributeMagnitudeWithBase(const FGameplayEffectCustomExecutionParameters& InParams,
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

FGameplayEffectCustomExecutionOutput UGGA_GameplayEffectCalculationFunctionLibrary::AddOutputModifier(FGameplayEffectCustomExecutionOutput& InExecutionOutput, FGameplayAttribute InAttribute,
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


void UGGA_GameplayEffectCalculationFunctionLibrary::MarkConditionalGameplayEffectsToTrigger(FGameplayEffectCustomExecutionOutput& InExecutionOutput)
{
	InExecutionOutput.MarkConditionalGameplayEffectsToTrigger();
}

void UGGA_GameplayEffectCalculationFunctionLibrary::MarkGameplayCuesHandledManually(FGameplayEffectCustomExecutionOutput& InExecutionOutput)
{
	InExecutionOutput.MarkGameplayCuesHandledManually();
}

void UGGA_GameplayEffectCalculationFunctionLibrary::MarkStackCountHandledManually(FGameplayEffectCustomExecutionOutput& InExecutionOutput)
{
	InExecutionOutput.MarkStackCountHandledManually();
}

FGameplayEffectContextHandle UGGA_GameplayEffectCalculationFunctionLibrary::GetEffectContextFromSpec(const FGameplayEffectSpec& EffectSpec)
{
	return EffectSpec.GetEffectContext();
}

void UGGA_GameplayEffectCalculationFunctionLibrary::AddAssetTagForPreMod(const FGameplayEffectCustomExecutionParameters& InParams, FGameplayTag NewGameplayTag)
{
	if (FGameplayEffectSpec* Spec = InParams.GetOwningSpecForPreExecuteMod())
	{
		Spec->AddDynamicAssetTag(NewGameplayTag);
	}
}

void UGGA_GameplayEffectCalculationFunctionLibrary::AddAssetTagsForPreMod(const FGameplayEffectCustomExecutionParameters& InParams, FGameplayTagContainer NewGameplayTags)
{
	if (FGameplayEffectSpec* Spec = InParams.GetOwningSpecForPreExecuteMod())
	{
		Spec->AppendDynamicAssetTags(NewGameplayTags);
	}
}

