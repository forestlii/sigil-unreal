// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCS_CombatStructLibrary.h"
#include "Engine/CancellableAsyncAction.h"
#include "GCS_AsyncAction_CollisionTrace.generated.h"

class UGCS_CollisionSystemComponent;
class UGCS_CollisionTraceInstance;

/**
 * Async action for setting up and listening to collision trace hits.
 * 设置并监听碰撞检测命中的异步动作。
 */
UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_AsyncAction_CollisionTrace : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Creates and activates trace instances from definitions and listens for hits.
	 * 从定义创建并激活碰撞检测实例并监听命中。
	 * @param CollisionSystem The collision system component. 碰撞系统组件。
	 * @param TraceDefinitions The trace definitions. 碰撞检测定义。
	 * @param PrimitiveComponent The primitive component for tracing. 用于追踪的原始组件。
	 * @return The async action instance. 异步动作实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGCS_AsyncAction_CollisionTrace* SetupAndListenForCollisionTraceHit(UGCS_CollisionSystemComponent* CollisionSystem, TArray<FGCS_CollisionTraceDefinition> TraceDefinitions,
	                                                                           UPrimitiveComponent* PrimitiveComponent);

	/**
	 * Activates the async action.
	 * 激活异步动作。
	 */
	virtual void Activate() override;

	/**
	 * Cancels the async action.
	 * 取消异步动作。
	 */
	virtual void Cancel() override;

	/**
	 * Delegate for collision trace hit events.
	 * 碰撞检测命中事件的委托。
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCollisionTraceSignature, UGCS_CollisionTraceInstance*, TraceInstance, const FHitResult&, HitResult);

	/**
	 * Called before trace instances are activated.
	 * 在激活碰撞检测实例前调用。
	 */
	UPROPERTY(BlueprintAssignable)
	FCollisionTraceSignature BeforeActive;

	/**
	 * Fired when a trace instance hits something.
	 * 当碰撞检测实例命中某物时触发。
	 */
	UPROPERTY(BlueprintAssignable)
	FCollisionTraceSignature OnHit;

protected:
	/**
	 * Handles trace instance hit events.
	 * 处理碰撞检测实例命中事件。
	 * @param TraceInstance The trace instance. 碰撞检测实例。
	 * @param HitResult The hit result. 命中结果。
	 */
	UFUNCTION()
	void TraceInstanceHitCallback(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult);

	/**
	 * The trace definitions for the action.
	 * 动作的碰撞检测定义。
	 */
	TArray<FGCS_CollisionTraceDefinition> TraceDefinitions;

	/**
	 * The collision system component.
	 * 碰撞系统组件。
	 */
	UPROPERTY()
	TWeakObjectPtr<UGCS_CollisionSystemComponent> CollisionSystemComponent;

	/**
	 * The primitive component for tracing.
	 * 用于追踪的原始组件。
	 */
	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> PrimitiveComponent;

	/**
	 * The active trace instances.
	 * 激活的碰撞检测实例。
	 */
	UPROPERTY()
	TArray<UGCS_CollisionTraceInstance*> TraceInstances;
};