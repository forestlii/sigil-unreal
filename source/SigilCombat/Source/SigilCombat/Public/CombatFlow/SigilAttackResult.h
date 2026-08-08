// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "SigilCombatStructLibrary.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "SigilAttackResult.generated.h"

class USigilAttackRequest_Melee;
class USigilCombatFlow;
class USigilCombatSystemComponent;

/**
 * Structure representing the result of a processed attack.
 * 表示已处理攻击结果的结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilAttackResult : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/**
	 * Gameplay tag indicating the impact result.
	 * 表示冲击结果的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGameplayTag ImpactResult;

	/**
	 * Array of tagged values associated with the attack.
	 * 与攻击相关的标记值数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	TArray<FSigilTaggedValue> TaggedValues;

	/**
	 * Optional object related to the attack.
	 * 与攻击相关的可选对象。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	TObjectPtr<const UObject> OptionalObject;

	/**
	 * Context handle for the gameplay effect.
	 * 游戏效果的上下文句柄。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGameplayEffectContextHandle EffectContextHandle;

	/**
	 * Aggregated source tags for the attack.
	 * 攻击的聚合来源标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGameplayTagContainer AggregatedSourceTags;

	/**
	 * Aggregated target tags for the attack.
	 * 攻击的聚合目标标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGameplayTagContainer AggregatedTargetTags;

	/**
	 * Whether the attack result has been consumed.
	 * 攻击结果是否已被消耗。
	 */
	UPROPERTY(NotReplicated)
	bool bConsumed{false};
};

/**
 * Container for storing combat results with network serialization.
 * 用于存储战斗结果的容器，支持网络序列化。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilAttackResultContainer : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	FSigilAttackResultContainer();

	/**
	 * Constructor with combat flow and max size.
	 * 带有战斗流程和最大尺寸的构造函数。
	 * @param InCombatFlow The combat flow instance. 战斗流程实例。
	 * @param InMaxSize The maximum size of the container. 容器最大尺寸。
	 */
	FSigilAttackResultContainer(USigilCombatFlow* InCombatFlow, int32 InMaxSize);

	/**
	 * Constructor with combat system component and max size.
	 * 带有战斗系统组件和最大尺寸的构造函数。
	 * @param InCombatSystemComponent The combat system component. 战斗系统组件。
	 * @param InMaxSize The maximum size of the container. 容器最大尺寸。
	 */
	FSigilAttackResultContainer(USigilCombatSystemComponent* InCombatSystemComponent, int32 InMaxSize);

	/**
	 * Sets the owning combat system component.
	 * 设置所属战斗系统组件。
	 * @param InCombatSystemComponent The combat system component. 战斗系统组件。
	 */
	void SetOwningCombatSystem(USigilCombatSystemComponent* InCombatSystemComponent) { CombatSystemComponent = InCombatSystemComponent; }

	/**
	 * Sets the combat flow.
	 * 设置战斗流程。
	 * @param InCombatFlow The combat flow instance. 战斗流程实例。
	 */
	void SetCombatFlow(USigilCombatFlow* InCombatFlow) { CombatFlow = InCombatFlow; }

	/**
	 * Adds a new attack result entry to the container.
	 * 向容器添加新的攻击结果条目。
	 * @param NewEntry The attack result to add. 要添加的攻击结果。
	 */
	void AddEntry(FSigilAttackResult& NewEntry);

	/**
	 * Handles post-replication addition of entries.
	 * 处理条目添加后的复制。
	 * @param AddedIndices The indices of added entries. 添加的条目索引。
	 * @param FinalSize The final size of the container. 容器最终尺寸。
	 */
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	/**
	 * Handles post-replication changes to entries.
	 * 处理条目更改后的复制。
	 * @param ChangedIndices The indices of changed entries. 更改的条目索引。
	 * @param FinalSize The final size of the container. 容器最终尺寸。
	 */
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

	/**
	 * Serializes the container for network replication.
	 * 为网络复制序列化容器。
	 * @param DeltaParms The network serialization parameters. 网络序列化参数。
	 * @return True if serialization is successful. 如果序列化成功返回true。
	 */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FSigilAttackResult, FSigilAttackResultContainer>(Results, DeltaParms, *this);
	}

private:
	/**
	 * Reference to the combat flow instance.
	 * 战斗流程实例的引用。
	 */
	UPROPERTY()
	TObjectPtr<USigilCombatFlow> CombatFlow;

	/**
	 * Reference to the combat system component.
	 * 战斗系统组件的引用。
	 */
	UPROPERTY()
	TObjectPtr<USigilCombatSystemComponent> CombatSystemComponent;

	/**
	 * List of attack results.
	 * 攻击结果列表。
	 */
	UPROPERTY()
	TArray<FSigilAttackResult> Results;

	/**
	 * Maximum size of the container.
	 * 容器最大尺寸。
	 */
	UPROPERTY()
	int32 MaxSize;
};

template <>
struct TStructOpsTypeTraits<FSigilAttackResultContainer> : public TStructOpsTypeTraitsBase2<FSigilAttackResultContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};