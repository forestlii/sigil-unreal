// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GGA_AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GGA_AbilitySystemFunctionLibrary.generated.h"

/**
 * Blueprint function library for ability system operations.
 * 用于技能系统操作的蓝图函数库。
 * @details Provides utility functions for managing ability system components, abilities, and attributes.
 * @细节 提供管理技能系统组件、技能和属性的实用函数。
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_AbilitySystemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region AbilitySystem
	/**
	 * Finds an ability system component on an actor.
	 * 在演员上查找技能系统组件。
	 * @param Actor The actor to search. 要搜索的演员。
	 * @param DesiredClass The desired class of the component. 所需的组件类。
	 * @param ASC The found ability system component (output). 找到的技能系统组件（输出）。
	 * @return True if the component was found, false otherwise. 如果找到组件则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem",
		meta=(DisplayName="Find Typed Ability System Component", DefaultToSelf="Actor", DynamicOutputParam="ASC", DeterminesOutputType="DesiredClass", ExpandBoolAsExecs="ReturnValue"))
	static bool FindAbilitySystemComponent(AActor* Actor, TSubclassOf<UAbilitySystemComponent> DesiredClass, UAbilitySystemComponent*& ASC);

	/**
	 * Retrieves an ability system component from an actor.
	 * 从演员获取技能系统组件。
	 * @param Actor The actor to search. 要搜索的演员。
	 * @param DesiredClass The desired class of the component. 所需的组件类。
	 * @return The found ability system component, or nullptr if not found. 找到的技能系统组件，未找到时为nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem",
		meta=(DisplayName="Get Typed Ability System Component", DefaultToSelf="Actor", DynamicOutputParam="ReturnValue", DeterminesOutputType="DesiredClass"))
	static UAbilitySystemComponent* GetAbilitySystemComponent(AActor* Actor, TSubclassOf<UAbilitySystemComponent> DesiredClass);

	/**
	 * Initializes the actor info for an ability system component.
	 * 初始化技能系统组件的演员信息。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param InOwnerActor The owner actor. 拥有者演员。
	 * @param InAvatarActor The avatar actor. 化身演员。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void InitAbilityActorInfo(UAbilitySystemComponent* AbilitySystem, AActor* InOwnerActor, AActor* InAvatarActor);

	/**
	 * Retrieves the owner actor of an ability system component.
	 * 获取技能系统组件的拥有者演员。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @return The owner actor. 拥有者演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static AActor* GetOwnerActor(UAbilitySystemComponent* AbilitySystem);

	/**
	 * Retrieves the avatar actor of an ability system component.
	 * 获取技能系统组件的化身演员。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @return The avatar actor. 化身演员。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static AActor* GetAvatarActor(UAbilitySystemComponent* AbilitySystem);

	/**
	 * Handles a gameplay event on an ability system component.
	 * 在技能系统组件上处理游戏事件。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param EventTag The event tag. 事件标签。
	 * @param Payload The event data payload. 事件数据负载。
	 * @return The number of successful ability activations. 成功激活的技能数量。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static int32 HandleGameplayEvent(UAbilitySystemComponent* AbilitySystem, FGameplayTag EventTag, const FGameplayEventData& Payload);

	/**
	 * Attempts to activate abilities one by one.
	 * 尝试逐一激活技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param AbilitiesToActivate Array of abilities to activate. 要激活的技能数组。
	 * @param bAllowRemoteActivation Whether to allow remote activation. 是否允许远程激活。
	 * @param bFirstOnly Whether to stop after the first successful activation. 是否在第一次成功激活后停止。
	 * @return True if at least one ability was activated, false otherwise. 如果至少激活一个技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem", meta=(DisplayName="Try Activate Abilities(one by one)"))
	static bool TryActivateAbilities(UAbilitySystemComponent* AbilitySystem, TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate, bool bAllowRemoteActivation = true, bool bFirstOnly = true);

	/**
	 * Retrieves activatable ability specs matching all tags.
	 * 获取与所有标签匹配的可激活技能规格。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Tags The tags to match. 要匹配的标签。
	 * @param MatchingGameplayAbilities Array to store matching ability specs (output). 存储匹配技能规格的数组（输出）。
	 * @param bOnlyAbilitiesThatSatisfyTagRequirements Whether to include only abilities satisfying tag requirements. 是否仅包括满足标签要求的技能。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void GetActivatableGameplayAbilitySpecsByAllMatchingTags(const UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& Tags,
	                                                                TArray<FGameplayAbilitySpecHandle>& MatchingGameplayAbilities, bool bOnlyAbilitiesThatSatisfyTagRequirements = true);

	/**
	 * Retrieves the first activatable ability matching all tags.
	 * 获取与所有标签匹配的第一个可激活技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Tags The tags to match. 要匹配的标签。
	 * @param MatchingGameplayAbility The matching ability spec handle (output). 匹配的技能规格句柄（输出）。
	 * @param bOnlyAbilitiesThatSatisfyTagRequirements Whether to include only abilities satisfying tag requirements. 是否仅包括满足标签要求的技能。
	 * @return True if a matching ability was found, false otherwise. 如果找到匹配技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "GGA|AbilitiesQuery", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool GetFirstActivatableAbilityByAllMatchingTags(const UAbilitySystemComponent* AbilitySystem, FGameplayTagContainer& Tags, FGameplayAbilitySpecHandle& MatchingGameplayAbility,
	                                                 bool bOnlyAbilitiesThatSatisfyTagRequirements = true);

	/**
	 * Retrieves active ability instances matching the specified tags.
	 * 获取与指定标签匹配的激活技能实例。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Tags The tags to match. 要匹配的标签。
	 * @param MatchingAbilityInstances Array to store matching ability instances (output). 存储匹配技能实例的数组（输出）。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void GetActiveAbilityInstancesWithTags(const UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& Tags, TArray<UGameplayAbility*>& MatchingAbilityInstances);

	/**
	 * Retrieves the ability instance playing the specified animation.
	 * 获取播放指定动画的技能实例。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @return The ability instance, or nullptr if not found. 技能实例，未找到时为nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static UGameplayAbility* GetAnimatingAbility(UAbilitySystemComponent* AbilitySystem);

	/**
	 * Checks if the specified ability is the current animating ability.
	 * 检查指定技能是否为当前播放动画的技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability to check. 要检查的技能。
	 * @return True if the ability is animating, false otherwise. 如果技能正在播放动画则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static bool IsAnimatingAbility(UAbilitySystemComponent* AbilitySystem, UGameplayAbility* Ability);

	/**
	 * Retrieves the ability instance playing the specified animation on an actor.
	 * 获取在演员上播放指定动画的技能实例。
	 * @param Actor The actor to search. 要搜索的演员。
	 * @param Animation The animation to check. 要检查的动画。
	 * @return The ability instance, or nullptr if not found. 技能实例，未找到时为nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem", meta=(DefaultToSelf="Actor"))
	static UGameplayAbility* GetAnimatingAbilityFromActor(AActor* Actor, UAnimSequenceBase* Animation);

	/**
	 * Finds the ability instance playing the specified animation with a desired class.
	 * 查找播放指定动画且具有所需类的技能实例。
	 * @param Actor The actor to search. 要搜索的演员。
	 * @param Animation The animation to check. 要检查的动画。
	 * @param DesiredClass The desired ability class. 所需的技能类。
	 * @param AbilityInstance The found ability instance (output). 找到的技能实例（输出）。
	 * @return True if an ability was found, false otherwise. 如果找到技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem",
		meta=(DefaultToSelf="Actor", DisplayName="Get Animating Ability from Actor", DeterminesOutputType=DesiredClass, DynamicOutputParam=AbilityInstance, ExpandBoolAsExecs=ReturnValue))
	static bool FindAnimatingAbilityFromActor(AActor* Actor, UAnimSequenceBase* Animation, TSubclassOf<UGameplayAbility> DesiredClass, UGameplayAbility*& AbilityInstance);

	/**
	 * Sends a gameplay event to an actor.
	 * 向演员发送游戏事件。
	 * @param Actor The actor to send the event to. 要发送事件的演员。
	 * @param EventTag The event tag. 事件标签。
	 * @param Payload The event data payload. 事件数据负载。
	 * @return True if any abilities were activated, false otherwise. 如果激活了任何技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem", Meta = (Tooltip = "This function can be used to trigger an ability on the actor in question with useful payload data."))
	static bool SendGameplayEventToActor(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload);

	/**
	 * Breaks down ability ended data into its components.
	 * 将技能结束数据分解为其组件。
	 * @param AbilityEndedData The ability ended data. 技能结束数据。
	 * @param AbilityThatEnded The ability that ended (output). 结束的技能（输出）。
	 * @param AbilitySpecHandle The ability spec handle (output). 技能规格句柄（输出）。
	 * @param bReplicateEndAbility Whether to replicate the end ability (output). 是否复制结束技能（输出）。
	 * @param bWasCancelled Whether the ability was cancelled (output). 技能是否被取消（输出）。
	 */
	UFUNCTION(BlueprintPure, Category = "GGA|AbilitySystem")
	static void BreakAbilityEndedData(const FAbilityEndedData& AbilityEndedData, UGameplayAbility*& AbilityThatEnded, FGameplayAbilitySpecHandle& AbilitySpecHandle, bool& bReplicateEndAbility,
	                                  bool& bWasCancelled);

	/**
	 * Finds all abilities matching the provided tags in order.
	 * 按顺序查找与提供的标签匹配的所有技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Tags The tags to match. 要匹配的标签。
	 * @param OutAbilityHandles Array to store matching ability handles (output). 存储匹配技能句柄的数组（输出）。
	 * @param bExactMatch Whether to require an exact tag match. 是否要求完全标签匹配。
	 * @return True if any abilities were found, false otherwise. 如果找到任何技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GGA|AbilitySystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool FindAllAbilitiesWithTagsInOrder(const UAbilitySystemComponent* AbilitySystem, TArray<FGameplayTag> Tags, TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles,
	                                            bool bExactMatch = true);

	/**
	 * Finds the first ability matching a gameplay tag query.
	 * 查找与游戏标签查询匹配的第一个技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param OutAbilityHandle The matching ability handle (output). 匹配的技能句柄（输出）。
	 * @param Query The gameplay tag query. 游戏标签查询。
	 * @return True if a matching ability was found, false otherwise. 如果找到匹配技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GGA|AbilitySystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool FindAbilityMatchingQuery(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle& OutAbilityHandle, FGameplayTagQuery Query);

	/**
	 * Finds the first ability matching the provided tags.
	 * 查找与提供的标签匹配的第一个技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param OutAbilityHandle The matching ability handle (output). 匹配的技能句柄（输出）。
	 * @param Tags The tags to match. 要匹配的标签。
	 * @param bExactMatch Whether to require an exact tag match. 是否要求完全标签匹配。
	 * @return True if a matching ability was found, false otherwise. 如果找到匹配技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GGA|AbilitySystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool FindAbilityWithTags(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle& OutAbilityHandle, FGameplayTagContainer Tags, bool bExactMatch = true);

	/**
	 * Adds a non-replicated gameplay tag to an ability system component.
	 * 向技能系统组件添加非复制的游戏标签。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param GameplayTag The tag to add. 要添加的标签。
	 * @param Count The number of times to add the tag. 添加标签的次数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void AddLooseGameplayTag(UAbilitySystemComponent* AbilitySystem, const FGameplayTag& GameplayTag, int32 Count = 1);

	/**
	 * Adds multiple non-replicated gameplay tags to an ability system component.
	 * 向技能系统组件添加多个非复制的游戏标签。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param GameplayTags The tags to add. 要添加的标签。
	 * @param Count The number of times to add the tags. 添加标签的次数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void AddLooseGameplayTags(UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& GameplayTags, int32 Count = 1);

	/**
	 * Removes a non-replicated gameplay tag from an ability system component.
	 * 从技能系统组件移除非复制的游戏标签。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param GameplayTag The tag to remove. 要移除的标签。
	 * @param Count The number of times to remove the tag. 移除标签的次数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void RemoveLooseGameplayTag(UAbilitySystemComponent* AbilitySystem, const FGameplayTag& GameplayTag, int32 Count = 1);

	/**
	 * Removes multiple non-replicated gameplay tags from an ability system component.
	 * 从技能系统组件移除多个非复制的游戏标签。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param GameplayTags The tags to remove. 要移除的标签。
	 * @param Count The number of times to remove the tags. 移除标签的次数。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void RemoveLooseGameplayTags(UAbilitySystemComponent* AbilitySystem, const FGameplayTagContainer& GameplayTags, int32 Count = 1);

	/**
	 * Removes all gameplay cues from an ability system component.
	 * 从技能系统组件移除所有游戏提示。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void RemoveAllGameplayCues(UAbilitySystemComponent* AbilitySystem);

#pragma endregion AbilitySystem

#pragma region Ability
	/**
	 * Sets the input pressed state for an ability.
	 * 为技能设置输入按下状态。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void SetAbilityInputPressed(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Sets the input released state for an ability.
	 * 为技能设置输入释放状态。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void SetAbilityInputReleased(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Checks if an ability can be activated.
	 * 检查技能是否可以激活。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param AbilityToActivate The ability to activate. 要激活的技能。
	 * @param RelevantTags Tags relevant to the activation check (output). 与激活检查相关的标签（输出）。
	 * @return True if the ability can be activated, false otherwise. 如果技能可以激活则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "GGA|AbilitySystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool CanActivateAbility(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle AbilityToActivate, FGameplayTagContainer& RelevantTags);

	/**
	 * Selects the first ability that can be activated from a list.
	 * 从列表中选择第一个可以激活的技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Abilities Array of abilities to check. 要检查的技能数组。
	 * @param OutAbilityHandle The selected ability handle (output). 选中的技能句柄（输出）。
	 * @return True if an ability was selected, false otherwise. 如果选中技能则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool SelectFirstCanActivateAbility(const UAbilitySystemComponent* AbilitySystem, TArray<FGameplayAbilitySpecHandle> Abilities, FGameplayAbilitySpecHandle& OutAbilityHandle);

	/**
	 * Cancels an ability.
	 * 取消一个技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle to cancel. 要取消的技能规格句柄。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void CancelAbility(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Cancels abilities based on tags.
	 * 根据标签取消技能。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param WithTags Tags to include for cancellation. 要包含的标签。
	 * @param WithoutTags Tags to exclude from cancellation. 要排除的标签。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|AbilitySystem")
	static void CancelAbilities(UAbilitySystemComponent* AbilitySystem, FGameplayTagContainer WithTags, FGameplayTagContainer WithoutTags);

	/**
	 * Checks if an ability instance is active.
	 * 检查技能实例是否激活。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return True if the ability instance is active, false otherwise. 如果技能实例激活则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static bool IsAbilityInstanceActive(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Retrieves ability instances for a spec handle.
	 * 获取技能规格句柄的技能实例。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return Array of ability instances. 技能实例数组。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static TArray<UGameplayAbility*> GetAbilityInstances(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Retrieves the ability CDO for a spec handle.
	 * 获取技能规格句柄的技能CDO。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return The ability CDO. 技能CDO。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static const UGameplayAbility* GetAbilityCDO(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Retrieves the level of an ability.
	 * 获取技能的等级。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return The ability level. 技能等级。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static int32 GetAbilityLevel(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Retrieves the input ID for an ability, if bound.
	 * 获取技能的输入ID（如果已绑定）。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return The input ID. 输入ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static int32 GetAbilityInputId(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Retrieves the source object for an ability.
	 * 获取技能的源对象。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return The source object. 源对象。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static UObject* GetAbilitySourceObject(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Retrieves dynamic tags for an ability.
	 * 获取技能的动态标签。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return The dynamic tag container. 动态标签容器。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static FGameplayTagContainer GetAbilityDynamicTags(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Retrieves the primary instance of an ability.
	 * 获取技能的主要实例。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return The primary ability instance. 主要技能实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static UGameplayAbility* GetAbilityPrimaryInstance(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

	/**
	 * Checks if an ability is active.
	 * 检查技能是否激活。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param Ability The ability spec handle. 技能规格句柄。
	 * @return True if the ability is active, false otherwise. 如果技能激活则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static bool IsAbilityActive(const UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Ability);

#pragma endregion Ability

#pragma region AttributeSet
	/**
	 * Finds an attribute set by class.
	 * 通过类查找属性集。
	 * @param AbilitySystem The ability system component. 技能系统组件。
	 * @param AttributeSetClass The attribute set class to find. 要查找的属性集类。
	 * @return The found attribute set, or nullptr if not found. 找到的属性集，未找到时为nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|AbilitySystem")
	static UAttributeSet* GetAttributeSetByClass(const UAbilitySystemComponent* AbilitySystem, const TSubclassOf<UAttributeSet> AttributeSetClass);
#pragma endregion

#pragma region ScalableFloat
	/**
	 * Retrieves the value of a scalable float at a specific level.
	 * 获取特定级别下可扩展浮点数的值。
	 * @param ScalableFloat The scalable float. 可扩展浮点数。
	 * @param Level The level to evaluate. 要评估的级别。
	 * @param ContextString Context string for debugging. 用于调试的上下文字符串。
	 * @return The evaluated value. 评估值。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|Extensions|ScalableFloat")
	static float GetValueAtLevel(const FScalableFloat& ScalableFloat, float Level, FString ContextString);
#pragma endregion
};