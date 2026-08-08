// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GGA_AbilitySystemEnumLibrary.h"
#include "GGA_AbilitySystemStructLibrary.h"
#include "GGA_GameplayAbilityInterface.h"
#include "Tickable.h"
#include "Abilities/GameplayAbility.h"
#include "GGA_GameplayAbility.generated.h"

class UGGA_AbilityCost;

DECLARE_STATS_GROUP(TEXT("GameplayAbility"), STATGROUP_GameplayAbility, STATCAT_Advanced)

/**
 * Extended gameplay ability class for custom functionality.
 * 扩展的游戏技能类，提供自定义功能。
 */
UCLASS(Abstract)
class GENERICGAMEPLAYABILITIES_API UGGA_GameplayAbility : public UGameplayAbility, public FTickableGameObject, public IGGA_GameplayAbilityInterface
{
	GENERATED_BODY()

	friend class UGGA_AbilitySystemComponent;

public:
	/**
	 * Constructor for the gameplay ability.
	 * 游戏技能构造函数。
	 */
	UGGA_GameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Updates the ability each frame.
	 * 每帧更新技能逻辑。
	 * @param DeltaTime Time since last frame. 自上一帧以来的时间。
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Retrieves the stat ID for performance tracking.
	 * 获取用于性能跟踪的统计ID。
	 * @return The stat ID. 统计ID。
	 */
	virtual TStatId GetStatId() const override;

	/**
	 * Checks if the ability can be ticked.
	 * 检查技能是否可被Tick。
	 * @return True if tickable, false otherwise. 如果可Tick则返回true，否则返回false。
	 */
	virtual bool IsTickable() const override;

	/**
	 * Blueprint event for per-frame ability updates.
	 * 每帧技能更新的蓝图事件。
	 * @param DeltaTime Time since last frame. 自上一帧以来的时间。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GGA|Ability")
	void AbilityTick(float DeltaTime);
	virtual void AbilityTick_Implementation(float DeltaTime);

	/**
	 * Checks if the input bound to this ability is pressed.
	 * 检查绑定到此技能的输入是否被按下。
	 * @return True if the input is pressed, false otherwise. 如果输入被按下则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Ability|Input")
	virtual bool IsInputPressed() const;

	/**
	 * Retrieves the controller associated with this ability.
	 * 获取与此技能相关的控制器。
	 * @return The controller. 控制器。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Ability")
	virtual AController* GetControllerFromActorInfo() const;

	/**
	 * Retrieves the activation group for this ability.
	 * 获取此技能的激活组。
	 * @return The activation group. 激活组。
	 */
	virtual EGGA_AbilityActivationGroup GetActivationGroup() const override { return ActivationGroup; }

	/**
	 * Sets the activation group for this ability.
	 * 设置此技能的激活组。
	 * @param NewGroup The new activation group. 新激活组。
	 */
	virtual void SetActivationGroup(EGGA_AbilityActivationGroup NewGroup) override;

	/**
	 * Attempts to activate the ability on spawn.
	 * 尝试在生成时激活技能。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param Spec The ability spec. 技能规格。
	 */
	virtual void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const override;

	/**
	 * Handles ability activation failure.
	 * 处理技能激活失败。
	 * @param FailedReason The reason for failure. 失败原因。
	 */
	virtual void HandleActivationFailed(const FGameplayTagContainer& FailedReason) const override;

	/**
	 * Checks if an effect container exists for a given tag.
	 * 检查是否存在指定标签的效果容器。
	 * @param ContainerTag The container tag to check. 要检查的容器标签。
	 * @return True if the container exists, false otherwise. 如果容器存在则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|Ability|EffectContainer")
	virtual bool HasEffectContainer(FGameplayTag ContainerTag);

	/**
	 * Creates an effect container spec from the effect container map.
	 * 从效果容器映射创建效果容器规格。
	 * @param ContainerTag The container tag. 容器标签。
	 * @param EventData The event data. 事件数据。
	 * @param OverrideGameplayLevel Optional gameplay level override. 可选的游戏等级覆盖。
	 * @return The effect container spec. 效果容器规格。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Ability|EffectContainer", meta = (AutoCreateRefTerm = "EventData"))
	virtual FGGA_GameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

	/**
	 * Applies an effect container by creating and applying its spec.
	 * 通过创建并应用规格来应用效果容器。
	 * @param ContainerTag The container tag. 容器标签。
	 * @param EventData The event data. 事件数据。
	 * @param OverrideGameplayLevel Optional gameplay level override. 可选的游戏等级覆盖。
	 * @return Array of active gameplay effect handles. 激活的游戏效果句柄数组。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Ability|EffectContainer", meta = (AutoCreateRefTerm = "EventData"))
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

protected:
	/**
	 * Activates the ability.
	 * 激活技能。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 * @param TriggerEventData Optional trigger event data. 可选的触发事件数据。
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * Prepares the ability for activation.
	 * 为技能激活做准备。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 * @param OnGameplayAbilityEndedDelegate Delegate for ability end. 技能结束委托。
	 * @param TriggerEventData Optional trigger event data. 可选的触发事件数据。
	 */
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;

	/**
	 * Ends the ability.
	 * 结束技能。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 * @param bReplicateEndAbility Whether to replicate ability end. 是否复制技能结束。
	 * @param bWasCancelled Whether the ability was cancelled. 技能是否被取消。
	 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/**
	 * Blueprint event for handling ability activation failure.
	 * 处理技能激活失败的蓝图事件。
	 * @param FailedReason The reason for failure. 失败原因。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Ability")
	void OnActivationFailed(const FGameplayTagContainer& FailedReason) const;

	/**
	 * Checks if the ability can be activated.
	 * 检查技能是否可以激活。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param SourceTags Optional source tags. 可选的源标签。
	 * @param TargetTags Optional target tags. 可选的目标标签。
	 * @param OptionalRelevantTags Optional relevant tags (output). 可选的相关标签（输出）。
	 * @return True if the ability can be activated, false otherwise. 如果技能可激活则返回true，否则返回false。
	 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	                                const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	/**
	 * Sets whether the ability can be canceled.
	 * 设置技能是否可被取消。
	 * @param bCanBeCanceled Whether the ability can be canceled. 技能是否可取消。
	 */
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;

	/**
	 * Called when the ability is granted to the ability system component.
	 * 技能被授予技能系统组件时调用。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param Spec The ability spec. 技能规格。
	 */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/**
	 * Blueprint event for when the ability is granted.
	 * 技能被授予时的蓝图事件。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", DisplayName = "On Give Ability")
	void K2_OnGiveAbility();

	/**
	 * Called when the ability is removed from the ability system component.
	 * 技能从技能系统组件移除时调用。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param Spec The ability spec. 技能规格。
	 */
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/**
	 * Blueprint event for when the ability is removed.
	 * 技能移除时的蓝图事件。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", DisplayName = "On Remove Ability")
	void K2_OnRemoveAbility();

	/**
	 * Called when the avatar is set for the ability.
	 * 技能的化身设置时调用。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param Spec The ability spec. 技能规格。
	 */
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/**
	 * Blueprint event for when the avatar is set.
	 * 化身设置时的蓝图事件。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", DisplayName = "On Avatar Set")
	void K2_OnAvatarSet();

	/**
	 * Checks if the ability should activate for a given network role.
	 * 检查技能是否应为特定网络角色激活。
	 * @param Role The network role. 网络角色。
	 * @return True if the ability should activate, false otherwise. 如果技能应激活则返回true，否则返回false。
	 */
	virtual bool ShouldActivateAbility(ENetRole Role) const override;

	/**
	 * Blueprint event for checking if the ability should activate for a network role.
	 * 检查技能是否应为网络角色激活的蓝图事件。
	 * @param Role The network role. 网络角色。
	 * @return True if the ability should activate, false otherwise. 如果技能应激活则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability", DisplayName = "Should Activate Ability")
	bool K2_ShouldActivateAbility(ENetRole Role) const;

	/**
	 * Handles input press for the ability.
	 * 处理技能的输入按下。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	/**
	 * Blueprint event for handling input press.
	 * 处理输入按下的蓝图事件。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability|Input", meta=(DisplayName="On Input Pressed"))
	void K2_OnInputPressed(const FGameplayAbilitySpecHandle Handle, FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	/**
	 * Handles input release for the ability.
	 * 处理技能的输入释放。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	/**
	 * Blueprint event for handling input release.
	 * 处理输入释放的蓝图事件。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability|Input", meta=(DisplayName="On Input Released"))
	void K2_OnInputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	/**
	 * Checks if the ability cost can be paid.
	 * 检查是否能支付技能成本。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param OptionalRelevantTags Optional relevant tags (output). 可选的相关标签（输出）。
	 * @return True if the cost can be paid, false otherwise. 如果成本可支付则返回true，否则返回false。
	 */
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/**
	 * Blueprint event for checking the ability cost.
	 * 检查技能成本的蓝图事件。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @return True if the cost can be paid, false otherwise. 如果成本可支付则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Cost", meta=(DisplayName="On Check Cost"))
	bool K2_OnCheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	virtual bool K2_OnCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	/**
	 * Applies the ability cost.
	 * 应用技能成本。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/**
	 * Retrieves the gameplay effect for the ability cost.
	 * 获取技能成本的游戏效果。
	 * @return The gameplay effect class. 游戏效果类。
	 */
	virtual UGameplayEffect* GetCostGameplayEffect() const override;

	/**
	 * Blueprint event for retrieving the cost gameplay effect.
	 * 获取成本游戏效果的蓝图事件。
	 * @return The gameplay effect class. 游戏效果类。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|Cost", meta=(DisplayName="Get Cost Gameplay Effect"))
	TSubclassOf<UGameplayEffect> K2_GetCostGameplayEffect() const;

	/**
	 * Blueprint event for applying additional ability costs.
	 * 应用额外技能成本的蓝图事件。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param ActorInfo The actor info. 演员信息。
	 * @param ActivationInfo The activation info. 激活信息。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Cost", meta=(DisplayName="On Apply Cost"))
	void K2_OnApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;
	virtual void K2_OnApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

	/**
	 * Applies ability tags to a gameplay effect spec.
	 * 将技能标签应用于游戏效果规格。
	 * @param Spec The gameplay effect spec. 游戏效果规格。
	 * @param AbilitySpec The ability spec. 技能规格。
	 */
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;

	/**
	 * Checks if the ability satisfies tag requirements.
	 * 检查技能是否满足标签要求。
	 * @param AbilitySystemComponent The ability system component. 技能系统组件。
	 * @param SourceTags Optional source tags. 可选的源标签。
	 * @param TargetTags Optional target tags. 可选的目标标签。
	 * @param OptionalRelevantTags Optional relevant tags (output). 可选的相关标签（输出）。
	 * @return True if requirements are satisfied, false otherwise. 如果满足要求则返回true，否则返回false。
	 */
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	                                               FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	/**
	 * Defines the activation group for the ability.
	 * 定义技能的激活组。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	EGGA_AbilityActivationGroup ActivationGroup;

	/**
	 * Additional costs required to activate the ability.
	 * 激活技能所需的额外成本。
	 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Costs")
	TArray<TObjectPtr<UGGA_AbilityCost>> AdditionalCosts;

	/**
	 * Loose tags applied to the owner while the ability is active.
	 * 技能激活时应用于所有者的松散标签。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	TArray<FGGA_GameplayTagCount> ActivationOwnedLooseTags;

	/**
	 * Map of gameplay effect containers, each associated with a tag.
	 * 游戏效果容器映射，每个容器与一个标签关联。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffects", meta=(ForceInlineRow))
	TMap<FGameplayTag, FGGA_GameplayEffectContainer> EffectContainerMap;

	/**
	 * Enables ticking for instanced-per-actor abilities when active.
	 * 为每演员实例化的技能启用Tick，仅在技能激活时有效。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	bool bEnableTick;

#pragma region Net
public:
	/**
	 * Attempts to activate the ability with batched RPCs.
	 * 尝试使用批处理RPC激活技能。
	 * @param InAbilityHandle The ability handle. 技能句柄。
	 * @param EndAbilityImmediately Whether to end the ability immediately. 是否立即结束技能。
	 * @return True if activation was successful, false otherwise. 如果激活成功则返回true，否则返回false。
	 */
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately) override;

	/**
	 * Ends the ability externally.
	 * 外部结束技能。
	 */
	virtual void ExternalEndAbility() override;

	/**
	 * Sends target data to the server from the predicting client.
	 * 从预测客户端向服务器发送目标数据。
	 * @param TargetData The target data to send. 要发送的目标数据。
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Net")
	virtual void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData);

#pragma endregion

#pragma region DataValidation
#if WITH_EDITOR
	/**
	 * Validates data in the editor.
	 * 在编辑器中验证数据。
	 * @param Context The data validation context. 数据验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	/**
	 * Pre-save processing for editor.
	 * 编辑器预保存处理。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
#pragma endregion
};