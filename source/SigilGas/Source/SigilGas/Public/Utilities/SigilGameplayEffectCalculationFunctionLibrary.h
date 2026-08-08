// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilGameplayEffectCalculationFunctionLibrary.generated.h"

/**
 * Blueprint function library for gameplay effect calculation operations.
 * 用于游戏效果计算操作的蓝图函数库。
 */
UCLASS()
class SIGILGAS_API USigilGameplayEffectCalculationFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the owning gameplay effect spec.
	 * 获取拥有的游戏效果规格。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The owning gameplay effect spec. 拥有的游戏效果规格。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static const FGameplayEffectSpec& GetOwningSpec(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Retrieves the effect context from execution parameters.
	 * 从执行参数获取效果上下文。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The effect context handle. 效果上下文句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static FGameplayEffectContextHandle GetEffectContext(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Retrieves the SetByCaller magnitude by tag from execution parameters.
	 * 从执行参数通过标签获取SetByCaller大小。
	 * @param InParams The execution parameters. 执行参数。
	 * @param Tag The tag to query. 要查询的标签。
	 * @param WarnIfNotFound Whether to warn if not found. 如果未找到是否警告。
	 * @param DefaultIfNotFound Default value if not found. 如果未找到的默认值。
	 * @return The magnitude value, or default if not found. 大小值，未找到时返回默认值。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GGA|Calculation")
	static float GetSetByCallerMagnitudeByTag(const FGameplayEffectCustomExecutionParameters& InParams, const FGameplayTag& Tag, bool WarnIfNotFound = false, float DefaultIfNotFound = 0.f);

	/**
	 * Retrieves the SetByCaller magnitude by name from execution parameters.
	 * 从执行参数通过名称获取SetByCaller大小。
	 * @param InParams The execution parameters. 执行参数。
	 * @param MagnitudeName The name to query. 要查询的名称。
	 * @param WarnIfNotFound Whether to warn if not found. 如果未找到是否警告。
	 * @param DefaultIfNotFound Default value if not found. 如果未找到的默认值。
	 * @return The magnitude value, or default if not found. 大小值，未找到时返回默认值。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GGA|Calculation")
	static float GetSetByCallerMagnitudeByName(const FGameplayEffectCustomExecutionParameters& InParams, const FName& MagnitudeName, bool WarnIfNotFound = false, float DefaultIfNotFound = 0.f);

	/**
	 * Retrieves source aggregated tags from the owning spec.
	 * 从拥有规格获取源聚合标签。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The source aggregated tags. 源聚合标签。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static FGameplayTagContainer GetSourceAggregatedTags(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Retrieves target aggregated tags from the owning spec.
	 * 从拥有规格获取目标聚合标签。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The target aggregated tags. 目标聚合标签。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static FGameplayTagContainer GetTargetAggregatedTags(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Retrieves the target ability system component.
	 * 获取目标技能系统组件。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The target ability system component. 目标技能系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static UAbilitySystemComponent* GetTargetASC(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Retrieves the target actor.
	 * 获取目标演员。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The target actor. 目标演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static AActor* GetTargetActor(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Retrieves the source ability system component.
	 * 获取源技能系统组件。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The source ability system component. 源技能系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static UAbilitySystemComponent* GetSourceASC(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Retrieves the source actor.
	 * 获取源演员。
	 * @param InParams The execution parameters. 执行参数。
	 * @return The source actor. 源演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static AActor* GetSourceActor(const FGameplayEffectCustomExecutionParameters& InParams);

	/**
	 * Attempts to calculate the magnitude of a captured attribute.
	 * 尝试计算捕获属性的值。
	 * @param InParams The execution parameters. 执行参数。
	 * @param InAttributeCaptureDefinitions Attribute capture definitions. 属性捕获定义。
	 * @param InAttribute The attribute to calculate. 要计算的属性。
	 * @param OutMagnitude The calculated magnitude (output). 计算出的大小（输出）。
	 * @return True if calculation was successful, false otherwise. 如果计算成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static bool AttemptCalculateCapturedAttributeMagnitude(const FGameplayEffectCustomExecutionParameters& InParams, TArray<FGameplayEffectAttributeCaptureDefinition> InAttributeCaptureDefinitions,
	                                                FGameplayAttribute InAttribute,
	                                                float& OutMagnitude);

	/**
	 * Attempts to calculate the magnitude of a captured attribute with source and target tags.
	 * 尝试使用源和目标标签计算捕获属性的值。
	 * @param InParams The execution parameters. 执行参数。
	 * @param SourceTags Source tags for the calculation. 用于计算的源标签。
	 * @param TargetTags Target tags for the calculation. 用于计算的目标标签。
	 * @param InAttributeCaptureDefinitions Attribute capture definitions. 属性捕获定义。
	 * @param InAttribute The attribute to calculate. 要计算的属性。
	 * @param OutMagnitude The calculated magnitude (output). 计算出的大小（输出）。
	 * @return True if calculation was successful, false otherwise. 如果计算成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static bool AttemptCalculateCapturedAttributeMagnitudeExt(const FGameplayEffectCustomExecutionParameters& InParams, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, TArray<FGameplayEffectAttributeCaptureDefinition> InAttributeCaptureDefinitions,
													FGameplayAttribute InAttribute,
													float& OutMagnitude);

	/**
	 * Attempts to calculate the magnitude of a captured attribute with a base value.
	 * 使用基础值尝试计算捕获属性的值。
	 * @param InParams The execution parameters. 执行参数。
	 * @param InAttributeCaptureDefinitions Attribute capture definitions. 属性捕获定义。
	 * @param InAttribute The attribute to calculate. 要计算的属性。
	 * @param InBaseValue The base value for the calculation. 计算的基础值。
	 * @param OutMagnitude The calculated magnitude (output). 计算出的大小（输出）。
	 * @return True if calculation was successful, false otherwise. 如果计算成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GGA|Calculation")
	static bool AttemptCalculateCapturedAttributeMagnitudeWithBase(const FGameplayEffectCustomExecutionParameters& InParams, TArray<FGameplayEffectAttributeCaptureDefinition> InAttributeCaptureDefinitions,
	                                                        FGameplayAttribute InAttribute, float InBaseValue, float& OutMagnitude);

	/**
	 * Adds an output modifier to the execution output.
	 * 将输出修改器添加到执行输出。
	 * @param InExecutionOutput The execution output to modify. 要修改的执行输出。
	 * @param InAttribute The attribute to modify. 要修改的属性。
	 * @param InModifierOp The modifier operation type. 修改器操作类型。
	 * @param InMagnitude The magnitude of the modifier. 修改器的大小。
	 * @return The modified execution output. 修改后的执行输出。
	 */
	UFUNCTION(BlueprintCallable, Category="GGA|Calculation")
	static FGameplayEffectCustomExecutionOutput AddOutputModifier(UPARAM(ref) FGameplayEffectCustomExecutionOutput& InExecutionOutput, FGameplayAttribute InAttribute,
	                                                              EGameplayModOp::Type InModifierOp, float InMagnitude);

	/**
	 * Marks conditional gameplay effects to trigger.
	 * 标记条件游戏效果以触发。
	 * @param InExecutionOutput The execution output to modify. 要修改的执行输出。
	 */
	UFUNCTION(BlueprintCallable, Category="GGA|Calculation")
	static void MarkConditionalGameplayEffectsToTrigger(UPARAM(ref) FGameplayEffectCustomExecutionOutput& InExecutionOutput);

	/**
	 * Marks gameplay cues as handled manually.
	 * 将游戏提示标记为手动处理。
	 * @param InExecutionOutput The execution output to modify. 要修改的执行输出。
	 */
	UFUNCTION(BlueprintCallable, Category="GGA|Calculation")
	static void MarkGameplayCuesHandledManually(UPARAM(ref) FGameplayEffectCustomExecutionOutput& InExecutionOutput);

	/**
	 * Marks the stack count as handled manually.
	 * 将堆叠计数标记为手动处理。
	 * @param InExecutionOutput The execution output to modify. 要修改的执行输出。
	 */
	UFUNCTION(BlueprintCallable, Category="GGA|Calculation")
	static void MarkStackCountHandledManually(UPARAM(ref) FGameplayEffectCustomExecutionOutput& InExecutionOutput);

	/**
	 * Retrieves the effect context from a gameplay effect spec.
	 * 从游戏效果规格获取效果上下文。
	 * @param EffectSpec The gameplay effect spec. 游戏效果规格。
	 * @return The effect context handle. 效果上下文句柄。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Calculation")
	static FGameplayEffectContextHandle GetEffectContextFromSpec(const FGameplayEffectSpec& EffectSpec);

	/**
	 * Adds a tag to the spec before applying modifiers.
	 * 在应用修改器前向规格添加标签。
	 * @param InParams The execution parameters. 执行参数。
	 * @param NewGameplayTag The tag to add. 要添加的标签。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Calculation")
	static void AddAssetTagForPreMod(const FGameplayEffectCustomExecutionParameters& InParams, FGameplayTag NewGameplayTag);

	/**
	 * Adds multiple tags to the spec before applying modifiers.
	 * 在应用修改器前向规格添加多个标签。
	 * @param InParams The execution parameters. 执行参数。
	 * @param NewGameplayTags The tags to add. 要添加的标签。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Calculation")
	static void AddAssetTagsForPreMod(const FGameplayEffectCustomExecutionParameters& InParams, FGameplayTagContainer NewGameplayTags);
};