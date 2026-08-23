// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Abilities/SigilAbilityEntitlementTypes.h"
#include "SigilAbilitySet.h"
#include "SigilAbilitySystemEnumLibrary.h"
#include "SigilAbilitySystemStructLibrary.h"
#include "SigilAbilitySystemComponent.generated.h"

class USigilAbilityTagRelationshipMapping;
class USigilGameplayAbility;
class USigilAbilitySet;

/**
 * Delegate for when the ability system is initialized.
 * 技能系统初始化时的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSigilAbilitySystemInitializedSignature);

/**
 * Delegate for when the ability system is uninitialized.
 * 技能系统取消初始化时的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSigilAbilitySystemUninitializedSignature);

/**
 * Delegate for when an ability is activated.
 * 技能激活时的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSigilOnAbilityActivatedSignature, const FGameplayAbilitySpecHandle, Handle, const UGameplayAbility*, Ability);

/**
 * Delegate for when an ability ends.
 * 技能结束时的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSigilOnAbilityEndedSignature, FGameplayAbilitySpecHandle, Handle, UGameplayAbility*, Ability, bool, bWasCancelled);

/**
 * Delegate for when an ability fails to activate.
 * 技能激活失败时的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSigilOnAbilityFailedSignature, const UGameplayAbility*, Ability, const FGameplayTagContainer&, ReasonTags);

/**
 * Extended ability system component with custom functionality.
 * 扩展的技能系统组件，包含自定义功能。
 */
UCLASS(ClassGroup=GGA, meta=(BlueprintSpawnableComponent))
class SIGILGAS_API USigilAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

#pragma region Common
public:
	/**
	 * Constructor for the ability system component.
	 * 技能系统组件构造函数。
	 */
	USigilAbilitySystemComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initializes the ability system with owner and avatar actors.
	 * 使用拥有者和化身演员初始化技能系统。
	 * @param InOwnerActor The owner actor. 拥有者演员。
	 * @param InAvatarActor The avatar actor. 化身演员。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Abilities")
	virtual void InitializeAbilitySystem(AActor* InOwnerActor, AActor* InAvatarActor);

	/**
	 * Uninitializes the ability system.
	 * 取消初始化技能系统。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Abilities")
	virtual void UninitializeAbilitySystem();

	/**
	 * Initializes actor info for the ability system.
	 * 初始化技能系统的演员信息。
	 * @param InOwnerActor The owner actor. 拥有者演员。
	 * @param InAvatarActor The avatar actor. 化身演员。
	 */
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	/**
	 * Delegate triggered when the ability system is initialized.
	 * 技能系统初始化时触发的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FSigilAbilitySystemInitializedSignature OnAbilitySystemInitialized;

	/**
	 * Delegate triggered when the ability system is uninitialized.
	 * 技能系统取消初始化时触发的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FSigilAbilitySystemUninitializedSignature OnAbilitySystemUninitialized;

	/**
	 * Called when the component ends play.
	 * 组件结束播放时调用。
	 * @param EndPlayReason The reason for ending play. 结束播放的原因。
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Initializes the component.
	 * 初始化组件。
	 */
	virtual void InitializeComponent() override;

	/**
	 * Initializes ability sets for the ability system.
	 * 为技能系统初始化技能集。
	 * @param InOwnerActor The owner actor. 拥有者演员。
	 * @param InAvatarActor The avatar actor. 化身演员。
	 */
	virtual void InitializeAbilitySets(AActor* InOwnerActor, AActor* InAvatarActor);

	/**
	 * Initializes attributes for the ability system.
	 * 为技能系统初始化属性。
	 * @param GroupName The attribute group name. 属性组名称。
	 * @param Level The level to initialize. 初始化等级。
	 * @param bInitialInit Whether this is the initial initialization. 是否为初始初始化。
	 */
	virtual void InitializeAttributes(FSigilAttributeGroupName GroupName, int32 Level, bool bInitialInit);

	/**
	 * Sends a replicated gameplay event to an actor.
	 * 向演员发送复制的游戏事件。
	 * @param Actor The target actor. 目标演员。
	 * @param EventTag The event tag. 事件标签。
	 * @param Payload The event data payload. 事件数据负载。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Abilities", meta=(DisplayName="Send Gameplay Event To Actor (Replicated)"))
	void SendGameplayEventToActor_Replicated(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload);

private:
	/**
	 * Server RPC for sending gameplay events.
	 * 发送游戏事件的服务器RPC。
	 * @param Actor The target actor. 目标演员。
	 * @param EventTag The event tag. 事件标签。
	 * @param Payload The event data payload. 事件数据负载。
	 */
	UFUNCTION(Server, Reliable, WithValidation, Category = "GGA|Abilities")
	void ServerSendGameplayEventToActor(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload);

	/**
	 * Multicast RPC for sending gameplay events.
	 * 发送游戏事件的多播RPC。
	 * @param Actor The target actor. 目标演员。
	 * @param EventTag The event tag. 事件标签。
	 * @param Payload The event data payload. 事件数据负载。
	 */
	UFUNCTION(NetMulticast, Reliable, Category = "GGA|Abilities")
	void MulticastSendGameplayEventToActor(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload);

protected:
	/**
	 * Post-initialization property setup.
	 * 初始化后属性设置。
	 */
	virtual void PostInitProperties() override;

	/**
	 * Registers the component with the global ability system.
	 * 将组件注册到全局技能系统。
	 */
	void RegisterToGlobalAbilitySystem();

	/**
	 * Unregisters the component from the global ability system.
	 * 从全局技能系统取消注册组件。
	 */
	void UnregisterToGlobalAbilitySystem();

	/**
	 * Default ability sets to grant on initialization.
	 * 初始化时赋予的默认技能集。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GGA|Abilities")
	TArray<TObjectPtr<const USigilAbilitySet>> DefaultAbilitySets;

	/**
	 * Attribute group name for initializing gameplay attributes.
	 * 用于初始化游戏属性的属性组名称。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GGA|Attributes")
	FSigilAttributeGroupName AttributeSetInitializeGroupName;

	/**
	 * Level for initializing attributes.
	 * 初始化属性的等级。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GGA|Attributes", meta=(ClampMin=1))
	int32 AttributeSetInitializeLevel{1};

private:
	/**
	 * Tracks if the ability system is initialized.
	 * 跟踪技能系统是否已初始化。
	 */
	UPROPERTY(VisibleInstanceOnly, Category = "GGA|Abilities")
	bool bAbilitySystemInitialized{false};

	/**
	 * Handles for granted default ability sets.
	 * 已赋予默认技能集的句柄。
	 */
	UPROPERTY(VisibleInstanceOnly, Category = "GGA|Abilities")
	TArray<FSigilAbilitySet_GrantedHandles> DefaultAbilitySet_GrantedHandles;

	/**
	 * Tracks if the component is registered with the global ability system.
	 * 跟踪组件是否注册到全局技能系统。
	 */
	UPROPERTY(VisibleInstanceOnly, Category = "GGA|Abilities")
	bool bRegisteredToGlobalAbilitySystem{false};

#pragma endregion

#pragma region AbilityEntitlements
public:
	/** Reconciles an already-loaded, ability-only entitlement snapshot on the authoritative ASC. 在权威 ASC 上对账已加载、仅含 Ability 的 entitlement 快照。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|Ability Entitlement")
	FSigilAbilityReconcileResult ReconcileAbilityEntitlements(const FSigilAbilityEntitlementSnapshot& Desired);

	/** Removes only the runtime grants and gate contributions owned by this entitlement projection. 只移除本投影拥有的运行时授予与激活门贡献。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|Ability Entitlement")
	void ResetAbilityEntitlementProjection();

	/** Adds or removes one idempotent activation-gate source. 增减一个幂等的激活门来源。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|Ability Entitlement")
	void SetAbilityActivationGateSource(FGameplayTag GateTag, FName SourceId, bool bPresent);

	/** Routes an exact dynamic spec source tag press to matching ability specs. 按精确动态来源 Tag 路由按下事件。 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Ability Entitlement")
	void AbilityInputTagPressed(FGameplayTag InputTag);

	/** Routes an exact dynamic spec source tag release to matching ability specs. 按精确动态来源 Tag 路由释放事件。 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Ability Entitlement")
	void AbilityInputTagReleased(FGameplayTag InputTag);

private:
	struct FAbilityEntitlementRuntimeGrant
	{
		FSigilAbilitySet_GrantedHandles GrantedHandles;
		TSet<FGameplayTag> Contributors;
		TMap<FString, FString> ExpectedSpecCanonicalEntries;
	};

	TMap<FString, FAbilityEntitlementRuntimeGrant> AbilityEntitlementRuntimeGrants;
	TMap<FGameplayTag, FString> AbilityEntitlementToIdentity;
	TMap<FGameplayTag, TSet<FName>> AbilityActivationGateSources;
	int64 AbilityEntitlementProjectionEpoch = 1;
	int64 LastAcceptedAbilityEntitlementRevision = INDEX_NONE;
	FString LastAcceptedAbilityEntitlementDigest;
	bool bHasAcceptedAbilityEntitlementSnapshot = false;
	bool bAbilityEntitlementReconcileInProgress = false;
	bool bAbilityEntitlementResetInProgress = false;
	bool bAbilityEntitlementProjectionNeedsReset = false;
	bool bAbilityActivationGateMutationInProgress = false;

#if WITH_DEV_AUTOMATION_TESTS
	int32 AbilityEntitlementFailGrantOrdinalForTest = INDEX_NONE;
#endif

	friend struct FSigilAbilityEntitlementProjectionTestAccess;

#pragma endregion

#pragma region AbilitiesActivation
public:
	/**
	 * Checks if an activation group is blocked.
	 * 检查激活组是否被阻挡。
	 * @param Group The activation group to check. 要检查的激活组。
	 * @return True if blocked, false otherwise. 如果被阻挡则返回true，否则返回false。
	 */
	bool IsActivationGroupBlocked(ESigilAbilityActivationGroup Group) const;

	/**
	 * Adds an ability to an activation group.
	 * 将技能添加到激活组。
	 * @param Group The activation group. 激活组。
	 * @param Ability The ability to add. 要添加的技能。
	 */
	void AddAbilityToActivationGroup(ESigilAbilityActivationGroup Group, UGameplayAbility* Ability);

	/**
	 * Removes an ability from an activation group.
	 * 从激活组移除技能。
	 * @param Group The activation group. 激活组。
	 * @param Ability The ability to remove. 要移除的技能。
	 */
	void RemoveAbilityFromActivationGroup(ESigilAbilityActivationGroup Group, UGameplayAbility* Ability);

	/**
	 * Checks if an ability can change its activation group.
	 * 检查技能是否可以更改激活组。
	 * @param NewGroup The new activation group. 新激活组。
	 * @param Ability The ability to check. 要检查的技能。
	 * @return True if the change is allowed, false otherwise. 如果允许更改则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GGA|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanChangeActivationGroup(ESigilAbilityActivationGroup NewGroup, UGameplayAbility* Ability) const;

	/**
	 * Changes the activation group of an ability.
	 * 更改技能的激活组。
	 * @param NewGroup The new activation group. 新激活组。
	 * @param Ability The ability to change. 要更改的技能。
	 * @return True if the change was successful, false otherwise. 如果更改成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GGA|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ChangeActivationGroup(ESigilAbilityActivationGroup NewGroup, UGameplayAbility* Ability);

	/**
	 * Delegate triggered when an ability is activated.
	 * 技能激活时触发的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FSigilOnAbilityActivatedSignature OnAbilityActivated;

	/**
	 * Delegate triggered when an ability fails to activate.
	 * 技能激活失败时触发的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FSigilOnAbilityFailedSignature OnAbilityActivationFailed;

protected:
	/**
	 * Notifies when an ability is activated.
	 * 技能激活时通知。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param Ability The activated ability. 激活的技能。
	 */
	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;

	/**
	 * Notifies when an ability fails to activate.
	 * 技能激活失败时通知。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param Ability The ability that failed. 失败的技能。
	 * @param FailureReason The reason for failure. 失败原因。
	 */
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;

	/**
	 * Client RPC for notifying ability activation failure.
	 * 通知技能激活失败的客户端RPC。
	 * @param Ability The ability that failed. 失败的技能。
	 * @param FailureReason The reason for failure. 失败原因。
	 */
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityActivationFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
	
	/**
	 * Implementation of client notification for ability activation failure.
	 * 技能激活失败客户端通知的实现。
	 */
	void ClientNotifyAbilityActivationFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	/**
	 * Handles ability activation failure.
	 * 处理技能激活失败。
	 * @param Ability The ability that failed. 失败的技能。
	 * @param FailureReason The reason for failure. 失败原因。
	 */
	virtual void HandleAbilityActivationFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	/**
	 * Tracks the number of abilities in each activation group.
	 * 跟踪每个激活组中的技能数量。
	 */
	int32 ActivationGroupCounts[(uint8)ESigilAbilityActivationGroup::MAX];

#pragma endregion

public:
	/**
	 * Function type for determining whether to cancel an ability.
	 * 确定是否取消技能的函数类型。
	 */
	typedef TFunctionRef<bool(const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;

	/**
	 * Cancels abilities based on a provided function.
	 * 根据提供的函数取消技能。
	 * @param ShouldCancelFunc Function to determine cancellation. 确定取消的函数。
	 * @param bReplicateCancelAbility Whether to replicate cancellation. 是否复制取消。
	 */
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	/**
	 * Cancels abilities in a specific activation group.
	 * 取消特定激活组中的技能。
	 * @param Group The activation group. 激活组。
	 * @param IgnoreAbility Ability to ignore. 要忽略的技能。
	 * @param bReplicateCancelAbility Whether to replicate cancellation. 是否复制取消。
	 */
	void CancelActivationGroupAbilities(ESigilAbilityActivationGroup Group, UGameplayAbility* IgnoreAbility, bool bReplicateCancelAbility);

	/**
	 * Delegate triggered when an ability ends.
	 * 技能结束时触发的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FSigilOnAbilityEndedSignature AbilityEndedEvent;

protected:
	/**
	 * Notifies when an ability ends.
	 * 技能结束时通知。
	 * @param Handle The ability spec handle. 技能规格句柄。
	 * @param Ability The ability that ended. 结束的技能。
	 * @param bWasCancelled Whether the ability was cancelled. 技能是否被取消。
	 */
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	/**
	 * Handles changes to whether an ability can be canceled.
	 * 处理技能是否可取消的更改。
	 * @param AbilityTags The ability tags. 技能标签。
	 * @param RequestingAbility The requesting ability. 请求的技能。
	 * @param bCanBeCanceled Whether the ability can be canceled. 技能是否可取消。
	 */
	virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

public:
	/**
	 * Retrieves cooldown information for specified tags.
	 * 获取指定标签的冷却信息。
	 * @param CooldownTags The cooldown tags. 冷却标签。
	 * @param TimeRemaining Remaining cooldown time (output). 剩余冷却时间（输出）。
	 * @param CooldownDuration Total cooldown duration (output). 总冷却持续时间（输出）。
	 * @return True if active cooldowns found, false otherwise. 如果找到激活的冷却则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Abilities")
	bool GetCooldownRemainingForTags(FGameplayTagContainer CooldownTags, float& TimeRemaining, float& CooldownDuration);

	/**
	 * Determines if server ability RPC batching is enabled.
	 * 确定是否启用服务器技能RPC批处理。
	 * @return True if batching is enabled. 如果启用批处理则返回true。
	 */
	virtual bool ShouldDoServerAbilityRPCBatch() const override { return true; }

	/**
	 * Attempts to activate an ability with batched RPCs.
	 * 尝试使用批处理RPC激活技能。
	 * @param InAbilityHandle The ability handle. 技能句柄。
	 * @param EndAbilityImmediately Whether to end the ability immediately. 是否立即结束技能。
	 * @return True if activation was successful, false otherwise. 如果激活成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Abilities")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);

protected:
	/**
	 * Gameplay effect replication mode.
	 * 游戏效果复制模式。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GGA|GameplayEffects")
	EGameplayEffectReplicationMode AbilitySystemReplicationMode;

public:
	/**
	 * Retrieves owned gameplay tags.
	 * 获取拥有的游戏标签。
	 * @param TagContainer The container for owned tags (output). 拥有的标签容器（输出）。
	 */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	/**
	 * Retrieves owned gameplay tags as a string.
	 * 以字符串形式获取拥有的游戏标签。
	 * @return The owned tags as a string. 拥有的标签字符串。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|GameplayTags")
	FString GetOwnedGameplayTagsString();

	/**
	 * Retrieves additional required and blocked activation tags.
	 * 获取额外的所需和阻止的激活标签。
	 * @param AbilityTags The ability tags. 技能标签。
	 * @param OutActivationRequired Required activation tags (output). 所需激活标签（输出）。
	 * @param OutActivationBlocked Blocked activation tags (output). 阻止激活标签（输出）。
	 */
	virtual void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

	/**
	 * Applies ability block and cancel tags.
	 * 应用技能阻挡和取消标签。
	 * @param AbilityTags The ability tags. 技能标签。
	 * @param RequestingAbility The requesting ability. 请求的技能。
	 * @param bEnableBlockTags Whether to enable block tags. 是否启用阻挡标签。
	 * @param BlockTags The block tags. 阻挡标签。
	 * @param bExecuteCancelTags Whether to execute cancel tags. 是否执行取消标签。
	 * @param CancelTags The cancel tags. 取消标签。
	 */
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags,
	                                            bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;

	/**
	 * Sets the tag relationship mapping.
	 * 设置标签关系映射。
	 * @param NewMapping The new tag relationship mapping. 新标签关系映射。
	 */
	UFUNCTION(BlueprintCallable, Category = "GGA|Gameplay Tags")
	void SetTagRelationshipMapping(USigilAbilityTagRelationshipMapping* NewMapping);

protected:
	/**
	 * Tag relationship mapping for ability activation and cancellation.
	 * 用于技能激活和取消的标签关系映射。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GGA|Gameplay Tags")
	TObjectPtr<USigilAbilityTagRelationshipMapping> TagRelationshipMapping;

public:
	/**
	 * Retrieves owned attribute sets as a string.
	 * 以字符串形式获取拥有的属性集。
	 * @return The owned attribute sets as a string. 拥有的属性集字符串。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GGA|Attributes")
	FString GetOwnedGameplayAttributeSetString();

#pragma region TargetData
public:
	/**
	 * Retrieves ability target data for a given ability handle and activation info.
	 * 获取指定技能句柄和激活信息的技能目标数据。
	 * @param AbilityHandle The ability handle. 技能句柄。
	 * @param ActivationInfo The activation info. 激活信息。
	 * @param OutTargetDataHandle The target data handle (output). 目标数据句柄（输出）。
	 */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);
#pragma endregion
};
