// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GGS_InteractionStructLibrary.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GGS_SmartObjectFunctionLibrary.generated.h"

class UGameplayInteractionSmartObjectBehaviorDefinition;
class UGameplayBehaviorSmartObjectBehaviorDefinition;
class UGameplayBehaviorConfig;
class USmartObjectBehaviorDefinition;

/**
 * Blueprint function library for smart object interactions.
 * 智能对象交互的蓝图函数库。
 */
UCLASS()
class GENERICGAMESYSTEM_API UGGS_SmartObjectFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the gameplay behavior config from a smart object behavior definition.
	 * 从智能对象行为定义获取游戏行为配置。
	 * @param BehaviorDefinition The smart object behavior definition. 智能对象行为定义。
	 * @return The gameplay behavior config. 游戏行为配置。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGS|SmartObject")
	static UGameplayBehaviorConfig* GetGameplayBehaviorConfig(const USmartObjectBehaviorDefinition* BehaviorDefinition);

	/**
	 * Finds a specific gameplay behavior config by class.
	 * 按类查找特定游戏行为配置。
	 * @param BehaviorDefinition The smart object behavior definition. 智能对象行为定义。
	 * @param DesiredClass The desired config class. 期望的配置类。
	 * @param OutConfig The found config (output). 找到的配置（输出）。
	 * @return True if the config was found, false otherwise. 如果找到配置返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGS|SmartObject", meta=(DeterminesOutputType="DesiredClass", DynamicOutputParam="OutConfig", ExpandBoolAsExecs="ReturnValue"))
	static bool FindGameplayBehaviorConfig(const USmartObjectBehaviorDefinition* BehaviorDefinition, TSubclassOf<UGameplayBehaviorConfig> DesiredClass, UGameplayBehaviorConfig*& OutConfig);

	/**
	 * Searches for smart object slots with interaction entrances on an actor.
	 * 在演员上搜索带有交互入口的智能对象槽。
	 * @param Filter The search filter. 搜索过滤器。
	 * @param SearchActor The actor to search. 要搜索的演员。
	 * @param OutResults The found smart object slot candidates (output). 找到的智能对象槽候选（输出）。
	 * @param UserActor Optional actor for additional data in condition evaluation. 用于条件评估的可选演员。
	 * @return True if at least one candidate was found, false otherwise. 如果找到至少一个候选返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = "GGS|SmartObject", Meta = (ReturnDisplayName = "bSuccess"))
	static bool FindSmartObjectsWithInteractionEntranceInActor(const FSmartObjectRequestFilter& Filter, AActor* SearchActor, TArray<FSmartObjectRequestResult>& OutResults,
	                                                           const AActor* UserActor = nullptr);

	/**
	 * Finds the interaction definition for a smart object slot.
	 * 查找智能对象槽的交互定义。
	 * @param WorldContext The world context object. 世界上下文对象。
	 * @param SmartObjectSlotHandle The smart object slot handle. 智能对象槽句柄。
	 * @param OutDefinition The interaction definition (output). 交互定义（输出）。
	 * @return True if the definition was found, false otherwise. 如果找到定义返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GGS|SmartObject", meta=(WorldContext="WorldContext", ExpandBoolAsExecs="ReturnValue"))
	static bool FindInteractionDefinitionFromSmartObjectSlot(UObject* WorldContext, FSmartObjectSlotHandle SmartObjectSlotHandle, UGGS_InteractionDefinition*& OutDefinition);
};
