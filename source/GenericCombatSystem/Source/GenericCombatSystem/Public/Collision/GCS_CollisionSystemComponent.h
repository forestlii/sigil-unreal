// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "GCS_CombatStructLibrary.h"
#include "GCS_CollisionSystemComponent.generated.h"

class UGCS_CollisionTraceInstance;
class UPrimitiveComponent;

/**
 * Cache for storing trace instances for reuse.
 * 用于存储可重用碰撞检测实例的缓存。
 */
USTRUCT()
struct GENERICCOMBATSYSTEM_API FGCS_TraceInstancesCache
{
	GENERATED_BODY()

	/**
	 * The cached trace instances.
	 * 缓存的碰撞检测实例。
	 */
	UPROPERTY(VisibleAnywhere, Category="GCS")
	TArray<TObjectPtr<UGCS_CollisionTraceInstance>> Instances;
};

/**
 * Component for managing collision trace instances.
 * 管理碰撞检测实例的组件。
 */
UCLASS(ClassGroup=(GCS), meta=(BlueprintSpawnableComponent), Blueprintable)
class GENERICCOMBATSYSTEM_API UGCS_CollisionSystemComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	UGCS_CollisionSystemComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Gets the collision system component from an actor.
	 * 从Actor获取碰撞系统组件。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @return The collision system component. 碰撞系统组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Collision", Meta = (DefaultToSelf="Actor"))
	static UGCS_CollisionSystemComponent* GetCollisionSystemComponent(const AActor* Actor);

	/**
	 * Finds the collision system component on an actor.
	 * 在Actor上查找碰撞系统组件。
	 * @param Actor The actor to query. 要查询的Actor。
	 * @param Component The found component (output). 找到的组件（输出）。
	 * @return True if found. 如果找到返回true。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Collision", Meta = (DefaultToSelf="Actor", ExpandBoolAsExecs="ReturnValue"))
	static bool FindCollisionSystemComponent(const AActor* Actor, UGCS_CollisionSystemComponent*& Component);

	/**
	 * Initializes the collision system.
	 * 初始化碰撞系统。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GIS|Collision", meta=(DisplayName="Initialize Collision System"))
	void OnInitialize();
	virtual void OnInitialize_Implementation();

	/**
	 * Deinitializes the collision system.
	 * 取消初始化碰撞系统。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GIS|Collision", meta=(DisplayName="Deinitialize Collision System"))
	void OnDeinitialize();
	virtual void OnDeinitialize_Implementation();

protected:
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
	 * Whether to auto-initialize on BeginPlay and deinitialize on EndPlay.
	 * 是否在BeginPlay时自动初始化，在EndPlay时自动取消初始化。
	 */
	UPROPERTY(EditAnywhere, Category="GCS|Collision")
	bool bAutoInitialize{true};

	/**
	 * Default trace definitions created on BeginPlay.
	 * 在BeginPlay时创建的默认碰撞检测定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GCS|Collision", meta=(TitleProperty="TraceTag"))
	TArray<FGCS_CollisionTraceDefinition> TraceDefinitions;

	/**
	 * Active trace instances.
	 * 激活的碰撞检测实例。
	 */
	UPROPERTY(VisibleAnywhere, Category = "GCS|Collision", Transient, meta=(ShowInnerProperties))
	TArray<TObjectPtr<UGCS_CollisionTraceInstance>> TraceInstances;

	/**
	 * Cached trace instances for reuse.
	 * 用于重用的缓存碰撞检测实例。
	 */
	UPROPERTY(VisibleAnywhere, Category = "GCS|Collision", Transient)
	TMap<TSubclassOf<UGCS_CollisionTraceInstance>, FGCS_TraceInstancesCache> CachedTraceInstances;

	/**
	 * Gets or creates a trace instance for the specified class.
	 * 获取或创建指定类的碰撞检测实例。
	 * @param TraceClass The trace instance class. 碰撞检测实例类。
	 * @return The trace instance. 碰撞检测实例。
	 */
	UGCS_CollisionTraceInstance* GetOrCreateTraceInstance(TSubclassOf<UGCS_CollisionTraceInstance> TraceClass);

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
	 * Creates trace instances from definitions.
	 * 从定义创建碰撞检测实例。
	 * @param Definitions The trace definitions. 碰撞检测定义。
	 * @param PrimitiveComponent The primitive component for tracing. 用于追踪的原始组件。
	 * @return The created trace instances. 创建的碰撞检测实例。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Collision", meta=(DisplayName="Create Trace Instances"))
	TArray<UGCS_CollisionTraceInstance*> CreateTraceInstances(TArray<FGCS_CollisionTraceDefinition> Definitions, UPrimitiveComponent* PrimitiveComponent);

	/**
	 * Gets all active trace instances.
	 * 获取所有激活的碰撞检测实例。
	 * @return The trace instances. 碰撞检测实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Collision")
	TArray<UGCS_CollisionTraceInstance*> GetTraceInstances() const;

	/**
	 * Gets a trace instance by tag.
	 * 通过标签获取碰撞检测实例。
	 * @param TraceToFind The tag to find. 要查找的标签。
	 * @param FoundTrace The found trace instance (output). 找到的碰撞检测实例（输出）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Collision")
	void GetTraceInstanceByTag(FGameplayTag TraceToFind, UGCS_CollisionTraceInstance*& FoundTrace);

	/**
	 * Gets a trace instance by class.
	 * 通过类获取碰撞检测实例。
	 * @param TraceToFind The class to find. 要查找的类。
	 * @param FoundTrace The found trace instance (output). 找到的碰撞检测实例（输出）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCS|Collision")
	void GetTraceInstanceByClass(TSubclassOf<UGCS_CollisionTraceInstance> TraceToFind, UGCS_CollisionTraceInstance*& FoundTrace);

	/**
	 * Toggles the state of trace instances based on tags.
	 * 根据标签切换碰撞检测实例的状态。
	 * @param TracesToControl The tags of traces to control. 要控制的碰撞检测标签。
	 * @param bNewState The new state. 新状态。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Collision")
	void ToggleTraceInstancesState(FGameplayTagContainer TracesToControl, bool bNewState);

	/**
	 * Removes a trace instance from the created traces.
	 * 从已创建的碰撞检测中移除实例。
	 * @param TraceToRemove The trace instance to remove. 要移除的碰撞检测实例。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Collision")
	void RemoveTraceFromCreatedTraces(UGCS_CollisionTraceInstance* TraceToRemove);

	/**
	 * Creates default trace instances based on TraceDefinitions.
	 * 根据TraceDefinitions创建默认碰撞检测实例。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Collision")
	virtual void CreateDefaultTraceInstances();

	/**
	 * Clears all active and cached trace instances.
	 * 清除所有激活和缓存的碰撞检测实例。
	 * @note OnTraceEndPlay event is not fired. OnTraceEndPlay事件不会触发。
	 */
	UFUNCTION(BlueprintCallable, Category = "GCS|Collision")
	void ClearTraceInstances();

	/**
	 * Delegate for trace instance hit events.
	 * 碰撞检测实例命中事件的委托。
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGCS_OnTraceInstanceHitSignature, UGCS_CollisionTraceInstance*, TraceInstance, const FHitResult&, HitResult);

	/**
	 * Event for trace instance hits.
	 * 碰撞检测实例命中事件。
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "GCS|Combat")
	FGCS_OnTraceInstanceHitSignature OnTraceInstanceHitEvent;

	/**
	 * Delegate for trace instance state change events.
	 * 碰撞检测实例状态更改事件的委托。
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGCS_OnTraceInstanceStateChangedSignature, UGCS_CollisionTraceInstance*, TraceInstance, bool, bNewState);

	/**
	 * Event for trace instance state changes.
	 * 碰撞检测实例状态更改事件。
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "GCS|Combat")
	FGCS_OnTraceInstanceStateChangedSignature OnTraceInstanceStateChangedEvent;

public:
	/**
	 * Ticks active trace instances.
	 * tick激活的碰撞检测实例。
	 * @param DeltaTime Time since last frame. 上一帧以来的时间。
	 */
	virtual void TickActiveTraces(float DeltaTime);

	/**
	 * Handles trace instance hit events.
	 * 处理碰撞检测实例命中事件。
	 * @param TraceInstance The trace instance. 碰撞检测实例。
	 * @param HitResult The hit result. 命中结果。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Collision")
	void OnTraceInstanceHit(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult);

	/**
	 * Handles trace instance state change events.
	 * 处理碰撞检测实例状态更改事件。
	 * @param TraceInstance The trace instance. 碰撞检测实例。
	 * @param bNewState The new state. 新状态。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Collision")
	void OnTraceInstanceStateChanged(UGCS_CollisionTraceInstance* TraceInstance, bool bNewState);
	
	//谁碰到了我
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "GCS|Combat")
	FGCS_OnTraceInstanceHitSignature OnBeTraceInstanceHitEvent;
	
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Collision")
	void OnBeTraceInstanceHit(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult);
};