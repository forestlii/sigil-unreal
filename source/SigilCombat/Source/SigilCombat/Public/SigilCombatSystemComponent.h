// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAbilitySystemGlobals.h"
#include "CombatFlow/SigilAttackResult.h"
#include "SigilCombatSystemComponent.generated.h"

class USigilCombatFlow;

/**
 * Structure for requesting montage playback.
 * 请求蒙太奇播放的结构。
 */
USTRUCT(BlueprintType)
struct SIGILCOMBAT_API FSigilPlayMontageRequest
{
	GENERATED_BODY()

	/**
	 * The animation montage to play.
	 * 要播放的动画蒙太奇。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GCS")
	TObjectPtr<UAnimMontage> AnimMontage{nullptr};

	/**
	 * The playback rate for the montage.
	 * 蒙太奇的播放速率。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GCS")
	float PlayRate{1.0f};

	/**
	 * The starting section name for the montage.
	 * 蒙太奇的起始片段名称。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GCS")
	FName StartSectionName{NAME_None};

	/**
	 * The scale for root motion translation.
	 * 根运动平移的缩放。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GCS")
	float RootTranslationScale{1.0f};

	/**
	 * The start time for the montage in seconds.
	 * 蒙太奇的起始时间（秒）。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GCS")
	float StartTimeSeconds{0.0f};

	/**
	 * Per-instigator request serial, assigned automatically by PlayPredictableMontageForTarget (0 = unassigned).
	 * Used to reconcile the client prediction with the server's accept/reject decision. Do not set by hand.
	 * 由 PlayPredictableMontageForTarget 自动分配的请求序号（0 = 未分配），用于把客户端预测与服务器的
	 * 接受/拒绝结果对账。不要手动设置。
	 */
	UPROPERTY(BlueprintReadOnly, Category="GCS")
	int32 RequestId{0};
};

/**
 * Structure for predicted montage information.
 * 预测蒙太奇信息的结构。
 */
USTRUCT()
struct FSigilPredictedMontageInfo
{
	GENERATED_BODY()

	/**
	 * The animation montage.
	 * 动画蒙太奇。
	 */
	UPROPERTY()
	TObjectPtr<UAnimMontage> AnimMontage{nullptr};

	/**
	 * The playback rate.
	 * 播放速率。
	 */
	UPROPERTY()
	float PlayRate{1.0f};

	/**
	 * The starting section name.
	 * 起始片段名称。
	 */
	UPROPERTY()
	FName StartSectionName{NAME_None};

	/**
	 * The time the montage was triggered.
	 * 蒙太奇触发的时间。
	 */
	UPROPERTY()
	float TriggeredTime{0.0f};

	/**
	 * Request serial this prediction belongs to (0 = no active prediction).
	 * 该预测对应的请求序号（0 = 当前无预测）。
	 */
	UPROPERTY()
	int32 RequestId{0};
};

/**
 * Structure for replicated montage information.
 * 复制蒙太奇信息的结构。
 */
USTRUCT()
struct FSigilReplicatedMontageInfo
{
	GENERATED_BODY()

	/**
	 * The animation montage.
	 * 动画蒙太奇。
	 */
	UPROPERTY()
	TObjectPtr<UAnimMontage> AnimMontage{nullptr};

	/**
	 * The playback rate.
	 * 播放速率。
	 */
	UPROPERTY()
	float PlayRate{1.0f};

	/**
	 * The starting section name.
	 * 起始片段名称。
	 */
	UPROPERTY()
	FName StartSectionName{NAME_None};

	/**
	 * The time the montage was triggered.
	 * 蒙太奇触发的时间。
	 */
	UPROPERTY()
	float TriggeredTime{0.0f};

	/**
	 * Montage-timeline position (seconds) the playback started from on the server.
	 * 服务器起播时所在的蒙太奇时间轴位置（秒）。
	 */
	UPROPERTY()
	float StartTimeSeconds{0.0f};

	/**
	 * Request serial of the accepted request (0 = none). Lets the predicting client match its prediction exactly.
	 * 被接受请求的序号（0 = 无），让预测端精确对账。
	 */
	UPROPERTY()
	int32 RequestId{0};

	/**
	 * Root motion translation scale applied on every machine while this montage plays.
	 * 该蒙太奇播放期间在每台机器上应用的根运动平移缩放。
	 */
	UPROPERTY()
	float RootTranslationScale{1.0f};
};

/**
 * Component for handling offensive and defensive combat behaviors.
 * 处理进攻和防御战斗行为的组件。
 */
UCLASS(ClassGroup=GCS, Blueprintable, BlueprintType, AutoExpandCategories=("GCS"), meta=(BlueprintSpawnableComponent))
class SIGILCOMBAT_API USigilCombatSystemComponent : public UActorComponent, public ISigilAbilitySystemGlobalsEventReceiver
{
	GENERATED_BODY()

	friend USigilCombatFlow;

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	USigilCombatSystemComponent();

	/**
	 * Initializes the component.
	 * 初始化组件。
	 */
	virtual void InitializeComponent() override;

	/**
	 * Called when the game starts.
	 * 游戏开始时调用。
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when the game ends.
	 * 游戏结束时调用。
	 * @param EndPlayReason The reason for ending. 结束原因。
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Retrieves lifetime replicated properties.
	 * 获取生命周期复制属性。
	 * @param OutLifetimeProps The lifetime properties. 生命周期属性。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Gets the combat system component for an actor.
	 * 获取演员的战斗系统组件。
	 * @param Actor The actor to query. 要查询的演员。
	 * @return The combat system component. 战斗系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Combat", Meta = (DefaultToSelf="Actor"))
	static USigilCombatSystemComponent* GetCombatSystemComponent(const AActor* Actor);

	/**
	 * Finds the combat system component for an actor.
	 * 查找演员的战斗系统组件。
	 * @param Actor The actor to query. 要查询的演员。
	 * @param CombatComponent The found component (output). 找到的组件（输出）。
	 * @return True if found. 如果找到返回true。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Combat", Meta = (DefaultToSelf="Actor", ExpandBoolAsExecs = "ReturnValue"))
	static bool FindCombatSystemComponent(const AActor* Actor, USigilCombatSystemComponent*& CombatComponent);

	/**
	 * Finds a typed combat system component for an actor.
	 * 查找演员的特定类型战斗系统组件。
	 * @param Actor The actor to query. 要查询的演员。
	 * @param DesiredClass The desired component class. 期望的组件类。
	 * @param Component The found component (output). 找到的组件（输出）。
	 * @return True if found. 如果找到返回true。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Combat", meta=(DefaultToSelf="Actor", DeterminesOutputType="DesiredClass", DynamicOutputParam="Component", ExpandBoolAsExecs="ReturnValue"))
	static bool FindTypedCombatSystemComponent(AActor* Actor, TSubclassOf<USigilCombatSystemComponent> DesiredClass, USigilCombatSystemComponent*& Component);

	/**
	 * Gets the combat flow for handling incoming attacks.
	 * 获取处理传入攻击的战斗流程。
	 * @return The combat flow instance. 战斗流程实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Combat", meta=(DisplayName="Get Combat Flow"))
	USigilCombatFlow* GetCombatFlow() const;

	/**
	 * True when the combat flow exists and has been initialized on this machine (server: after creation;
	 * client: after OnRep_CombatFlow). Attack results are only dispatched once this is true.
	 * 战斗流程存在且已在本机初始化（服务器：创建后；客户端：OnRep_CombatFlow 后）时为 true。
	 * 攻击结果只在此之后分发。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Combat")
	bool IsCombatFlowReady() const;

	/**
	 * Registers an attack result.
	 * 注册攻击结果。
	 * @param Payload The attack result to register. 要注册的攻击结果。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GCS|Combat")
	void RegisterAttackResult(UPARAM(ref) FSigilAttackResult& Payload);

	/**
	 * Gets the last processed attack result.
	 * 获取最后处理的攻击结果。
	 * @return The last attack result. 最后攻击结果。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Combat")
	FSigilAttackResult GetLastProcessedAttackResult() const;

	/**
	 * Sets the last processed attack result.
	 * 设置最后处理的攻击结果。
	 * @param Payload The attack result to set. 要设置的攻击结果。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Combat")
	void SetLastProcessedAttackResult(const FSigilAttackResult& Payload);

	/**
	 * Plays a predictable montage for a target combat system component.
	 * 为目标战斗系统组件播放可预测的蒙太奇。
	 * @param TargetCSC The target combat system component. 目标战斗系统组件。
	 * @param Request The montage play request. 蒙太奇播放请求。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Combat")
	void PlayPredictableMontageForTarget(USigilCombatSystemComponent* TargetCSC, FSigilPlayMontageRequest Request);

	/**
	 * Server RPC to play a predictable montage for a target. Not meant to be called directly — use
	 * PlayPredictableMontageForTarget, which assigns the RequestId and predicts locally.
	 * 为目标播放可预测蒙太奇的服务器 RPC。不建议直接调用——请用 PlayPredictableMontageForTarget，
	 * 它会分配 RequestId 并在本地预测。
	 * @param TargetCSC The target combat system component. 目标战斗系统组件。
	 * @param Request The montage play request. 蒙太奇播放请求。
	 */
	UFUNCTION(Server, Reliable, Category="GCS|Combat")
	void ServerPlayPredictableMontageForTarget(USigilCombatSystemComponent* TargetCSC, FSigilPlayMontageRequest Request);

	/**
	 * Client RPC: the server rejected the request with this id; roll back the local prediction on its target.
	 * 客户端 RPC：服务器拒绝了该序号的请求，回滚其目标上的本地预测。
	 * @param RequestId The rejected request serial. 被拒绝的请求序号。
	 */
	UFUNCTION(Client, Reliable)
	void ClientMontageRequestRejected(int32 RequestId);

	/**
	 * Structural validation of a montage request: non-null montage, finite play rate within the configured
	 * min/max, finite non-negative root translation scale, start time inside the montage, existing section,
	 * and a *linear* montage (no looping / non-linear section graph — predictable playback derives clear time
	 * and late-join position from linear time). Runs on every role; never trusts the caller.
	 * 蒙太奇请求的结构校验：蒙太奇非空、播放倍率有限且在配置的上下限内、根运动缩放有限非负、
	 * 起始时间在蒙太奇内、Section 存在，且蒙太奇为**线性**（无循环 / 非线性 Section 图——可预测播放按
	 * 线性时间推导清理时机与延迟补播位置）。所有角色都执行，不信任调用方。
	 * @param TargetCSC The component the montage would be applied to. 蒙太奇作用的目标组件。
	 * @param Request The request to validate. 待校验请求。
	 * @param OutReason Optional human-readable rejection reason. 可选的拒绝原因。
	 * @return True if the request is well-formed. 请求合法则返回 true。
	 */
	virtual bool IsMontageRequestValid(const USigilCombatSystemComponent* TargetCSC, const FSigilPlayMontageRequest& Request, FString* OutReason = nullptr) const;

	/**
	 * Returns true if every montage section either ends the montage or links to the next section in order.
	 * 若每个 Section 要么结束蒙太奇、要么按顺序衔接下一个 Section，则返回 true。
	 */
	static bool IsMontageLinear(const UAnimMontage* Montage);

	/**
	 * Authorization hook evaluated on the server before a montage request from this component is applied to a target.
	 * Default implementation: the component may always target itself; any other target is rejected unless
	 * USigilCombatSystemSettings::MaxPredictableMontageTargetDistance is greater than zero *and* the target lives in the
	 * same world within that distance. Cross-target playback is therefore opt-in. Override to enforce team,
	 * attack-flow-state or other project rules.
	 * 服务器在把本组件发出的蒙太奇请求应用到目标前调用的授权钩子。默认实现：本组件可以始终作用于自身；
	 * 其它目标一律拒绝，除非 USigilCombatSystemSettings::MaxPredictableMontageTargetDistance 大于 0 **且**目标位于
	 * 同一世界、在该距离内。因此跨目标播放需要显式开启。覆写以加入阵营、攻击流程状态或其它项目规则。
	 * @param TargetCSC The target combat system component. 目标战斗系统组件。
	 * @param Request The montage play request. 蒙太奇播放请求。
	 * @return True if this component may apply the montage to the target. 允许则返回 true。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GCS|Combat")
	bool CanPlayMontageOnTarget(const USigilCombatSystemComponent* TargetCSC, const FSigilPlayMontageRequest& Request) const;
	virtual bool CanPlayMontageOnTarget_Implementation(const USigilCombatSystemComponent* TargetCSC, const FSigilPlayMontageRequest& Request) const;

	/**
	 * Sets the replicated montage information. Caller must have validated the request.
	 * 设置复制的蒙太奇信息。调用方须已校验请求。
	 * @param Request The montage play request. 蒙太奇播放请求。
	 */
	void SetReplicatedMontage(const FSigilPlayMontageRequest& Request);

	/**
	 * Timer that clears ReplicatedMontageInfo once the current montage has finished.
	 * 当前蒙太奇播完后清空 ReplicatedMontageInfo 的计时器。
	 */
	FTimerHandle MontageClearTimerHandle;

	/**
	 * Monotonic serial of montage requests; guards the clear timer against stale callbacks.
	 * 蒙太奇请求的单调序号，用于防止过期计时器回调清掉新请求。
	 */
	uint32 MontageRequestSerial{0};

	/**
	 * Handles replication of montage information.
	 * 处理蒙太奇信息的复制。
	 */
	UFUNCTION()
	void OnRep_ReplicatedMontageInfo();

	/**
	 * Plays a predicted montage locally. Prediction state is only recorded if the montage actually started.
	 * 在本地播放预测蒙太奇。仅当蒙太奇确实开始播放时才记录预测状态。
	 * @param Request The montage play request. 蒙太奇播放请求。
	 * @return True if the montage started. 蒙太奇已开始播放则返回 true。
	 */
	bool PlayPredictedMontage(const FSigilPlayMontageRequest& Request);

	/**
	 * Stops and forgets the local prediction with the given request id (no-op if it is not the active prediction).
	 * 停止并清除指定序号的本地预测（若它不是当前预测则无操作）。
	 */
	void CancelPredictedMontage(int32 RequestId);

	/**
	 * Applies a root-motion translation scale to the owning character (no-op for non-character owners).
	 * 对拥有者角色应用根运动平移缩放（非角色拥有者无操作）。
	 */
	void ApplyRootTranslationScale(float Scale) const;

private:
	/** Server-side per-second request counter for rate limiting. 服务器侧每秒请求计数（限频用）。 */
	int32 MontageRequestsInWindow{0};
	double MontageRequestWindowStart{0.0};

	/** Instigator-side (client) book-keeping: outstanding predictions by request id → target. 发起端（客户端）待对账预测：请求序号 → 目标。 */
	struct FPendingPrediction
	{
		TWeakObjectPtr<USigilCombatSystemComponent> Target;
		double IssuedTime{0.0};
	};
	TMap<int32, FPendingPrediction> PendingPredictions;
	int32 NextMontageRequestId{1};

public:

	/**
	 * Gets the character's skeletal mesh component.
	 * 获取角色的骨骼网格组件。
	 * @return The skeletal mesh component. 骨骼网格组件。
	 */
	USkeletalMeshComponent* GetCharacterMeshComponent() const;

protected:
	/**
	 * Handles pre-gameplay effect spec application.
	 * 处理游戏效果规格应用前逻辑。
	 * @param Spec The gameplay effect spec. 游戏效果规格。
	 * @param AbilitySystemComponent The ability system component. 能力系统组件。
	 */
	virtual void OnGlobalPreGameplayEffectSpecApply(FGameplayEffectSpec& Spec, UAbilitySystemComponent* AbilitySystemComponent) override;

	/**
	 * Handles replication of the combat flow.
	 * 处理战斗流程的复制。
	 */
	UFUNCTION()
	void OnRep_CombatFlow();

	/**
	 * The class of the combat flow to instantiate.
	 * 要实例化的战斗流程类。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCS|Combat Settings")
	TSubclassOf<USigilCombatFlow> CombatFlowClass;

	/**
	 * The instantiated combat flow.
	 * 实例化的战斗流程。
	 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_CombatFlow, Category = "GCS|Combat State", meta=(ShowInnerProperties))
	TObjectPtr<USigilCombatFlow> CombatFlow;

	/**
	 * The last attack result processed by the combat flow.
	 * 战斗流程处理的最后攻击结果。
	 */
	UPROPERTY(VisibleAnywhere, Category = "GCS|Combat State")
	FSigilAttackResult LastProcessedAttackResult;

	/**
	 * Container for attack results.
	 * 攻击结果容器。
	 */
	UPROPERTY(VisibleAnywhere, Replicated, Category="GCS|Combat State")
	FSigilAttackResultContainer AttackResultContainer;

	/**
	 * Replicated montage information.
	 * 复制的蒙太奇信息。
	 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ReplicatedMontageInfo, Category = "GCS|Combat State")
	FSigilReplicatedMontageInfo ReplicatedMontageInfo;

	/**
	 * Predicted montage information.
	 * 预测的蒙太奇信息。
	 */
	UPROPERTY(VisibleAnywhere, Category = "GCS|Combat State")
	FSigilPredictedMontageInfo PredictedMontageInfo;
};