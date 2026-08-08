// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilInputFunctionLibrary.generated.h"

/**
 * Utility functions for the Generic Input System.
 * 通用输入系统的实用功能。
 */
UCLASS()
class SIGILINPUT_API USigilInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the input action value from the action instance.
	 * 从输入动作实例中获取输入动作值。
	 * @param ActionDataData The input action instance data. 输入动作实例数据。
	 * @return The input action value. 输入动作值。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIPS|Input", meta = (BlueprintAutocast))
	static FInputActionValue GetInputActionValue(const FInputActionInstance& ActionDataData);

	/**
	 * Gets the last tag name from a gameplay tag.
	 * 从游戏标签中获取最后一个标签名称。
	 * @param Tag The gameplay tag. 游戏标签。
	 * @return The last tag name. 最后一个标签名称。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GIPS|Utilities", meta=(DisplayName="Get Last Tag Name(GIPS)"))
	static FName GetLastTagName(FGameplayTag Tag);

	/**
	 * Converts a gameplay tag container to a simple string.
	 * 将游戏标签容器转换为简单字符串。
	 * @param Tags The gameplay tag container. 游戏标签容器。
	 * @return The string representation of the tags. 标签的字符串表示。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GIPS|Utilities", meta=(DisplayName="Get Simple String Of Tags(GIPS)"))
	static FString GetSimpleStringOfTags(FGameplayTagContainer Tags);

	/**
	 * Gets an array of last tag names from a tag container.
	 * 从标签容器中获取最后一个标签名称的数组。
	 * @param Tags The gameplay tag container. 游戏标签容器。
	 * @return Array of last tag names. 最后一个标签名称的数组。
	 */
	static TArray<FName> GetLastTagNameArray(FGameplayTagContainer Tags);

	/**
	 * Gets a string of last tag names from a tag container.
	 * 从标签容器中获取最后一个标签名称的字符串。
	 * @param Tags The gameplay tag container. 游戏标签容器。
	 * @return String of last tag names. 最后一个标签名称的字符串。
	 */
	static FString GetLastTagNameString(FGameplayTagContainer Tags);

	/**
	 * Gets a description of a gameplay tag query.
	 * 获取游戏标签查询的描述。
	 * @param TagQuery The gameplay tag query. 游戏标签查询。
	 * @return The description of the tag query. 标签查询的描述。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GIPS|Utilities", meta=(DisplayName="Get TagQuery Description(GIPS)"))
	static FString GetTagQueryDescription(const FGameplayTagQuery& TagQuery);

	/**
	 * Converts a trigger event to a string.
	 * 将触发事件转换为字符串。
	 * @param TriggerEvent The trigger event. 触发事件。
	 * @return The string representation of the trigger event. 触发事件的字符串表示。
	 */
	static FString GetTriggerEventString(ETriggerEvent TriggerEvent);
};