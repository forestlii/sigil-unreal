// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "SmartObjectRuntime.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectTypes.h"
#if ENGINE_MINOR_VERSION >= 6
#include "SmartObjectRequestTypes.h"
#endif
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "SigilInteractionStructLibrary.generated.h"

class USigilInteractionSystemComponent;
class UAbilitySystemComponent;
class USigilInteractionDefinition;

/**
 * Structure wrapping an interaction definition for smart object interaction.
 * 封装智能对象交互的交互定义结构。
 */
USTRUCT(DisplayName="Interaction Entrance")
struct SIGILINTERACTION_API FSigilSmartObjectInteractionEntranceData : public FSmartObjectDefinitionData
{
	GENERATED_BODY()

	/**
	 * Interaction definition containing static data for player interaction.
	 * 包含玩家交互静态数据的交互定义。
	 * @note Replicated across the network.
	 * @注意 通过网络同步。
	 */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(DisplayName="Definition"))
	TSoftObjectPtr<USigilInteractionDefinition> DefinitionDA{nullptr};
};

/**
 * Structure representing an interaction option.
 * 表示交互选项的结构。
 */
USTRUCT(BlueprintType)
struct SIGILINTERACTION_API FSigilInteractionOption
{
	GENERATED_BODY()

	/**
	 * Interaction definition associated with this option.
	 * 与此选项关联的交互定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TObjectPtr<USigilInteractionDefinition> Definition{nullptr};

	/**
	 * Smart object request result for this option. Not replicated.
	 * 此选项的智能对象请求结果。未网络同步。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NotReplicated, Category="Interaction")
	FSmartObjectRequestResult RequestResult;

	/**
	 * Smart object behavior definition for this option.
	 * 此选项的智能对象行为定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NotReplicated, Category="Interaction")
	TObjectPtr<const USmartObjectBehaviorDefinition> BehaviorDefinition;

	/**
	 * Index of the associated smart object slot, used for UI sorting.
	 * 关联智能对象槽的索引，用于UI排序。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	int32 SlotIndex{-1};

	/**
	 * State of the associated smart object slot, used for UI input rules.
	 * 关联智能对象槽的状态，用于UI输入规则。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	ESmartObjectSlotState SlotState{ESmartObjectSlotState::Free};

	/**
	 * Equality operator for comparing interaction options.
	 * 交互选项的相等比较运算符。
	 */
	friend bool operator==(const FSigilInteractionOption& Lhs, const FSigilInteractionOption& RHS);

	/**
	 * Inequality operator for comparing interaction options.
	 * 交互选项的不等比较运算符。
	 */
	friend bool operator!=(const FSigilInteractionOption& Lhs, const FSigilInteractionOption& RHS);

	/**
	 * Less-than operator for sorting interaction options by slot index.
	 * 按槽索引排序交互选项的比较运算符。
	 */
	friend bool operator<(const FSigilInteractionOption& Lhs, const FSigilInteractionOption& RHS);

	/**
	 * Converts the interaction option to a string representation.
	 * 将交互选项转换为字符串表示。
	 * @return String representation of the option. 选项的字符串表示。
	 */
	FString ToString() const;
};