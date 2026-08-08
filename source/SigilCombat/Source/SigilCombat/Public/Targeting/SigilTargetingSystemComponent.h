// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "SigilTargetingSystemComponent.generated.h"

class UTargetingPreset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilOnTargetLockOnSignature, AActor*, NewTargetActor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilOnTargetLockOffSignature, AActor*, PrevTargetActor);

/**
 * Component for managing combat targeting.
 * 管理战斗目标的组件。
 */
UCLASS(ClassGroup=(GCS), AutoExpandCategories=("GCS"), meta=(BlueprintSpawnableComponent), Blueprintable)
class SIGILCOMBAT_API USigilTargetingSystemComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	USigilTargetingSystemComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Gets the targeting system component from an actor.
	 * 从Actor获取目标系统组件。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @return The targeting system component. 目标系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Targeting", Meta = (DefaultToSelf="Actor"))
	static USigilTargetingSystemComponent* GetTargetingSystemComponent(const AActor* Actor);

	/**
	 * Retrieves lifetime replicated properties.
	 * 获取生命周期复制属性。
	 * @param OutLifetimeProps The lifetime properties. 生命周期属性。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/**
	 * Called when the game starts.
	 * 游戏开始时调用。
	 */
	virtual void BeginPlay() override;

	/**
	 * The currently targeted actor.
	 * 当前目标Actor。
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "GCS|Targeting")
	TObjectPtr<AActor> TargetedActor = nullptr;

	/**
	 * List of potential target actors (server-side only).
	 * 潜在目标Actor列表（仅限服务器）。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GCS|Targeting", meta=(AllowPrivateAccess=true))
	TArray<TObjectPtr<AActor>> PotentialTargets;

	/**
	 * Whether to automatically update potential targets based on tick rate.
	 * 是否根据tick频率自动更新潜在目标。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS|Targeting", meta=(AllowPrivateAccess=true))
	bool bAutoUpdatePotentialTargets{true};

	/**
	 * Targeting preset for searching and filtering targets.
	 * 用于搜索和过滤目标的目标预设。
	 * @note The component's owner is the SourceActor, and the component is the SourceObject.
	 * @注意 组件的Owner作为SourceActor，组件本身作为SourceObject。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GCS|Targeting", meta=(AllowPrivateAccess=true))
	TObjectPtr<UTargetingPreset> TargetingPreset;

public:
	/**
	 * Called every frame.
	 * 每帧调用。
	 * @param DeltaTime Time since last frame. 上一帧以来的时间。
	 * @param TickType The type of tick. tick类型。
	 * @param ThisTickFunction The tick function. tick函数。
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Refreshes targeting based on the delta time.
	 * 根据时间增量刷新目标。
	 * @param DeltaTime Time since last frame. 上一帧以来的时间。
	 */
	virtual void RefreshTargeting(float DeltaTime);

	/**
	 * Forces collection of potential targets and selects the best one.
	 * 强制收集潜在目标并选择最佳目标。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Targeting")
	void SearchForActorToTarget();

	/**
	 * Selects the closest actor from potential targets within a radius.
	 * 从潜在目标中选择指定范围内最近的Actor。
	 * @param Radius The search radius. 搜索半径。
	 * @return The closest actor. 最近的Actor。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Targeting")
	AActor* SelectClosestActorFromPotentialTargets(float Radius = 100) const;

	/**
	 * Filters actors using a targeting preset.
	 * 使用目标预设过滤Actor。
	 * @param InTargetingPreset The targeting preset. 目标预设。
	 * @param InTargets The input targets. 输入目标。
	 * @param OutActors The filtered actors (output). 过滤后的Actor（输出）。
	 * @return True if filtering succeeded. 如果过滤成功返回true。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Targeting", meta=(ExpandBoolAsExecs="ReturnValue"))
	bool FilterActorsWithPreset(UTargetingPreset* InTargetingPreset, const TArray<AActor*> InTargets, TArray<AActor*>& OutActors);

	/**
	 * Selects a target from potential targets.
	 * 从潜在目标中选择一个目标。
	 */
	virtual void SelectFromPotentialTargets();

	/**
	 * Switches to a new target based on direction.
	 * 根据方向切换到新目标。
	 * @param RightDirection Whether to switch to the right. 是否向右切换。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Targeting")
	void StaticSwitchToNewTarget(bool RightDirection);

	/**
	 * Refreshes the list of potential targets using the targeting preset.
	 * 使用目标预设刷新潜在目标列表。
	 */
	virtual void RefreshPotentialTargets();

	/**
	 * Checks if an actor can be targeted.
	 * 检查Actor是否可以被目标。
	 * @param ActorToTarget The actor to check. 要检查的Actor。
	 * @return True if the actor can be targeted. 如果Actor可被目标返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Targeting")
	bool CanBeTargeted(AActor* ActorToTarget);
	virtual bool CanBeTargeted_Implementation(AActor* ActorToTarget);

	/**
	 * Gets the currently targeted actor.
	 * 获取当前目标Actor。
	 * @return The targeted actor. 目标Actor。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Targeting")
	FORCEINLINE AActor* GetTargetedActor() { return TargetedActor; }

	/**
	 * Sets the targeted actor.
	 * 设置目标Actor。
	 * @param NewActor The new target actor. 新目标Actor。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Targeting")
	void SetTargetedActor(AActor* NewActor);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSigilOnRefreshAssassinationSignature);
	
	UPROPERTY(BlueprintAssignable, Category="GCS|Assassination")
	FSigilOnRefreshAssassinationSignature OnRefreshAssassinationEvent;

	//暗杀
	UFUNCTION(BlueprintCallable, Category = "GCS|Assassination")
	void AddAssassinationListen();
	
	UFUNCTION(BlueprintCallable, Category = "GCS|Assassination")
	void RemoveAssassinationListen();
private:
	/**
	 * Sets the targeted actor with optional RPC.
	 * 设置目标Actor，可选择是否发送RPC。
	 * @param NewActor The new target actor. 新目标Actor。
	 * @param bSendRpc Whether to send RPC. 是否发送RPC。
	 */
	void SetTargetedActor(AActor* NewActor, bool bSendRpc);

	/**
	 * Client RPC to set the targeted actor.
	 * 设置目标Actor的客户端RPC。
	 * @param NewActor The new target actor. 新目标Actor。
	 */
	UFUNCTION(Client, Reliable)
	void ClientSetTargetedActor(AActor* NewActor);
	virtual void ClientSetTargetedActor_Implementation(AActor* NewActor);

	/**
	 * Server RPC to set the targeted actor.
	 * 设置目标Actor的服务器RPC。
	 * @param NewActor The new target actor. 新目标Actor。
	 */
	UFUNCTION(Server, Reliable)
	void ServerSetTargetedActor(AActor* NewActor);
	virtual void ServerSetTargetedActor_Implementation(AActor* NewActor);
	// 暗杀引用计数
	int CheckAssassinationTargetRefCount = 0;
	bool CheckAssassination() const;
protected:
	/**
	 * Handles target lock-off events.
	 * 处理目标解锁事件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Targeting")
	void OnLockOff();
	virtual void OnLockOff_Implementation();

	/**
	 * Handles target lock-on events.
	 * 处理目标锁定事件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Targeting")
	void OnLockOn();
	virtual void OnLockOn_Implementation();

	/**
	 * Delegate for target lock-on events.
	 * 目标锁定事件的委托。
	 */
	UPROPERTY(BlueprintAssignable, Category="GCS|Targeting")
	FSigilOnTargetLockOnSignature OnTargetLockOnEvent;

	/**
	 * Delegate for target lock-off events.
	 * 目标解锁事件的委托。
	 */
	UPROPERTY(BlueprintAssignable, Category="GCS|Targeting")
	FSigilOnTargetLockOffSignature OnTargetLockOffEvent;

	/**
	 * Calculates the view angle to a target actor.
	 * 计算到目标Actor的视角角度。
	 * @param TargetActor The target actor. 目标Actor。
	 * @return The view angle. 视角角度。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Targeting")
	virtual float CalculateViewAngle(const AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Assassination")
	void OnRefreshAssassination();
	
	virtual void OnRefreshAssassination_Implementation();
};
