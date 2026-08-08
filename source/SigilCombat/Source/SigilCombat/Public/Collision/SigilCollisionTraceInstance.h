// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SigilActorOwnedObject.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "SigilCollisionTraceInstance.generated.h"

class UPrimitiveComponent;
class USigilAttackRequest_Base;
class UTargetingPreset;
class USigilAttackRequest_Melee;
class USigilCollisionSystemComponent;

/**
 * Delegate for trace hit events.
 * 碰撞检测命中事件的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilOnTraceHitSignature, const FHitResult&, HitResult);

/**
 * Delegate for trace state change events.
 * 碰撞检测状态更改事件的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilOnTraceStateChangedSignature, bool, NewState);

/**
 * Object for managing collision trace instances.
 * 管理碰撞检测实例的对象。
 */
UCLASS(Blueprintable, AutoExpandCategories = ("GCS"))
class SIGILCOMBAT_API USigilCollisionTraceInstance : public USigilActorOwnedObject
{
	GENERATED_BODY()

	friend USigilCollisionSystemComponent;

public:
	/**
	 * Gameplay tag for the trace instance.
	 * 碰撞检测实例的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GCS|Trace Settings", meta=(ExposeOnSpawn))
	FGameplayTag TraceGameplayTag;

	/**
	 * The primitive component used for tracing.
	 * 用于追踪的原始组件。
	 */
	UPROPERTY(BlueprintReadWrite, Category = "GCS|Trace Settings", meta=(ExposeOnSpawn))
	TObjectPtr<UPrimitiveComponent> TracePrimitiveComponent;

	/**
	 * Socket names on the primitive component for tracing.
	 * 原始组件上用于追踪的插槽名称。
	 */
	UPROPERTY(BlueprintReadWrite, Category = "GCS|Trace Settings", meta=(ExposeOnSpawn))
	TArray<FName> TracePrimitiveComponentSocketNames;

	/**
	 * Targeting preset for fetching target actors.
	 * 用于获取目标Actor的目标预设。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="GCS|Trace Settings",meta=(ExposeOnSpawn))
	TObjectPtr<UTargetingPreset> TargetingPreset;

	/**
	 * The actor that created this trace instance.
	 * 创建此碰撞检测实例的Actor。
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GCS|Trace Settings")
	TObjectPtr<AActor> TraceOwner = nullptr;

	/**
	 * Associated information for this trace.
	 * 此碰撞检测的关联信息。
	 */
	UPROPERTY(VisibleAnywhere, Transient, Category = "GCS|Trace State")
	FGameplayEventData TraceInformation;

	/**
	 * Delegate for trace hit events.
	 * 碰撞检测命中事件的委托。
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSigilOnTraceHitSignature OnHit;

	/**
	 * Delegate for trace state change events.
	 * 碰撞检测状态更改事件的委托。
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSigilOnTraceStateChangedSignature OnTraceStateChangedEvent;

	/**
	 * The active duration of the trace instance.
	 * 碰撞检测实例的激活时间。
	 */
	UPROPERTY(BlueprintReadOnly, Category="GCS|Trace State")
	float ActiveTime{0.0f};

	/**
	 * Broadcasts a hit event.
	 * 广播命中事件。
	 * @param HitResult The hit result. 命中结果。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Trace")
	void BroadcastHit(const FHitResult& HitResult);

	/**
	 * Broadcasts a state change event.
	 * 广播状态更改事件。
	 * @param bNewState The new state. 新状态。
	 */
	void BroadcastStateChanged(bool bNewState);

protected:
	/**
	 * Initializes the trace instance.
	 * 初始化碰撞检测实例。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Trace", meta=(BlueprintProtected))
	void OnTraceBeginPlay();
	virtual void OnTraceBeginPlay_Implementation();

	/**
	 * Cleans up the trace instance.
	 * 清理碰撞检测实例。
	 * @note The instance is returned to the cache pool instead of being destroyed.
	 * @注意 实例被返回到缓存池而不是销毁。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Trace", meta=(BlueprintProtected))
	void OnTraceEndPlay();
	virtual void OnTraceEndPlay_Implementation();

	/**
	 * Handles trace ticking.
	 * 处理碰撞检测的tick。
	 * @param DeltaSeconds Time since last frame. 上一帧以来的时间。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Trace", meta=(BlueprintProtected))
	void OnTraceTick(float DeltaSeconds);
	virtual void OnTraceTick_Implementation(float DeltaSeconds);

	/**
	 * Handles trace state changes.
	 * 处理碰撞检测状态更改。
	 * @param bNewState The new state. 新状态。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Trace", meta=(BlueprintProtected))
	void OnTraceStateChanged(bool bNewState);
	virtual void OnTraceStateChanged_Implementation(bool bNewState);

	/**
	 * Actors hit during the active duration.
	 * 激活期间命中的Actor。
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="GCS|Trace State", AdvancedDisplay)
	TArray<TObjectPtr<AActor>> HitActors;

public:
	/**
	 * Sets the trace mesh information.
	 * 设置碰撞检测网格信息。
	 * @param NewPrimitiveComponent The new primitive component. 新原始组件。
	 * @param PrimitiveComponentSocketNames The socket names. 插槽名称。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Trace")
	void SetTraceMeshInfo(UPrimitiveComponent* NewPrimitiveComponent, TArray<FName> PrimitiveComponentSocketNames);

	/**
	 * Checks if an actor can be hit.
	 * 检查是否可以命中Actor。
	 * @param ActorToCheck The actor to check. 要检查的Actor。
	 * @return True if the actor can be hit. 如果可以命中返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GCS|Trace")
	bool CanHitActor(const AActor* ActorToCheck) const;
	bool CanHitActor_Implementation(const AActor* ActorToCheck) const;

	/**
	 * Gets the source actor for the trace.
	 * 获取碰撞检测的源Actor。
	 * @return The source actor (e.g., weapon, bullet, or trace owner). 源Actor（例如武器、子弹或TraceOwner）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Trace")
	AActor* GetTraceSourceActor() const;

	/**
	 * Toggles the trace state.
	 * 切换碰撞检测状态。
	 * @param bNewState The new state (active traces tick and attempt to hit). 新状态（激活的追踪会tick并尝试命中）。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Trace")
	void ToggleTraceState(bool bNewState);

	/**
	 * Indicates if the trace is active.
	 * 表示碰撞检测是否激活。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCS|Trace")
	bool bTraceActive{false};

protected:
	/**
	 * Handles trace hit events.
	 * 处理碰撞检测命中事件。
	 * @param HitResult The hit result. 命中结果。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GCS|Trace", meta=(BlueprintProtected))
	void OnTraceHit(const FHitResult& HitResult);
	virtual void OnTraceHit_Implementation(const FHitResult& HitResult);
};