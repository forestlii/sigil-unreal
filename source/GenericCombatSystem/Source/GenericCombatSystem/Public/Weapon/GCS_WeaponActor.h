// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GCS_CombatStructLibrary.h"
#include "GCS_WeaponInterface.h"
#include "GameFramework/Actor.h"
#include "GCS_WeaponActor.generated.h"

/**
 * Delegate for weapon active state changes.
 * 武器激活状态更改的委托。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGCS_WeaponActiveStateChangedSignature, bool, bIsActive);

/**
 * Default implementation of the weapon interface as an actor.
 * 作为Actor的武器接口默认实现。
 * @note Extend this class for custom weapon logic.
 * @注意 扩展此类以实现自定义武器逻辑。
 */
UCLASS(BlueprintType, Blueprintable, Abstract, ClassGroup=(GCS))
class GENERICCOMBATSYSTEM_API AGCS_WeaponActor : public AActor, public IGCS_WeaponInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	AGCS_WeaponActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Gets the pawn owning this weapon.
	 * 获取拥有此武器的Pawn。
	 * @return The owning pawn. 所属Pawn。
	 */
	virtual APawn* GetWeaponOwner_Implementation() const override;

	/**
	 * Gets the gameplay tags associated with the weapon.
	 * 获取与武器关联的游戏标签。
	 * @return The weapon's gameplay tags. 武器游戏标签。
	 */
	virtual const FGameplayTagContainer GetWeaponTags_Implementation() const override;

	/**
	 * Retrieves lifetime replicated properties.
	 * 获取生命周期复制属性。
	 * @param OutLifetimeProps The lifetime properties. 生命周期属性。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Returns this actor.
	 * 返回此Actor。
	 * @return The actor. Actor。
	 */
	virtual AActor* AsActor_Implementation() const override;

	/**
	 * Sets the weapon's active state.
	 * 设置武器的激活状态。
	 * @param bNewActive The new active state. 新激活状态。
	 */
	virtual void SetWeaponActive_Implementation(bool bNewActive) override;

	/**
	 * Checks if the weapon is active.
	 * 检查武器是否激活。
	 * @return True if the weapon is active. 如果武器激活返回true。
	 */
	virtual bool IsWeaponActive_Implementation() const override;

	/**
	 * Gets the main primitive component of the weapon.
	 * 获取武器的主要原始组件。
	 * @return The primitive component. 原始组件。
	 */
	virtual UPrimitiveComponent* GetPrimitiveComponent_Implementation() override;

	/**
	 * Gets the source object associated with the weapon.
	 * 获取与武器关联的源对象。
	 * @return The source object. 源对象。
	 */
	virtual UObject* GetSourceObject_Implementation() const override;

	/**
	 * Sets the source object for the weapon.
	 * 设置武器的源对象。
	 * @param NewSourceObject The new source object. 新源对象。
	 */
	virtual void SetSourceObject_Implementation(UObject* NewSourceObject) override;

	/**
	 * Gets the owned gameplay tags.
	 * 获取拥有的游戏标签。
	 * @param TagContainer The gameplay tag container (output). 游戏标签容器（输出）。
	 */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

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
	 * Handles weapon active state changes.
	 * 处理武器激活状态变化。
	 * @param Prev The previous active state. 之前的激活状态。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Weapon")
	void OnWeaponActiveStateChanged(bool Prev);

	/**
	 * Handles source object changes.
	 * 处理源对象变化。
	 * @param Prev The previous source object. 之前的源对象。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GCS|Weapon")
	void OnSourceObjectChanged(const UObject* Prev);
	virtual void OnSourceObjectChanged_Implementation(const UObject* Prev);

	/**
	 * Refreshes trace instances and registers/unregisters trace events.
	 * 刷新碰撞检测实例并注册/取消注册碰撞事件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|WeaponTrace", meta=(BlueprintProtected))
	void RefreshTraceInstance();
	virtual void RefreshTraceInstance_Implementation();

	/**
	 * Handles weapon trace hits.
	 * 处理武器碰撞命中。
	 * @param TraceInstance The collision trace instance. 碰撞检测实例。
	 * @param HitResult The hit result. 命中结果。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|WeaponTrace")
	void OnAnyTraceHit(UGCS_CollisionTraceInstance* TraceInstance, const FHitResult& HitResult);

	/**
	 * Handles trace state changes.
	 * 处理碰撞状态变化。
	 * @param TraceInstance The collision trace instance. 碰撞检测实例。
	 * @param NewState The new state. 新状态。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|WeaponTrace")
	void OnAnyTraceStateChanged(UGCS_CollisionTraceInstance* TraceInstance, bool NewState);

	/**
	 * Delegate for weapon active state changes.
	 * 武器激活状态更改的委托。
	 */
	UPROPERTY(BlueprintAssignable)
	FGCS_WeaponActiveStateChangedSignature OnWeaponActiveStateChangedEvent;

public:
	/**
	 * Called every frame.
	 * 每帧调用。
	 * @param DeltaSeconds Time since last frame. 上一帧以来的时间。
	 */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/**
	 * Gameplay tags for the weapon.
	 * 武器的游戏标签。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSetting")
	FGameplayTagContainer WeaponTags;

	/**
	 * List of collision trace settings created when the weapon is activated.
	 * 武器激活时创建的碰撞检测设置列表。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSetting|Trace")
	TArray<FGCS_CollisionTraceDefinition> TraceDefinitions;

	/**
	 * The source object associated with the weapon.
	 * 与武器关联的源对象。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="WeaponState", ReplicatedUsing = OnSourceObjectChanged)
	TObjectPtr<UObject> SourceObject;

	/**
	 * List of active trace instances.
	 * 激活的碰撞检测实例列表。
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WeaponState")
	TArray<TObjectPtr<UGCS_CollisionTraceInstance>> TraceInstances;

	/**
	 * Indicates if the weapon is active.
	 * 表示武器是否激活。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = OnWeaponActiveStateChanged, Category = "WeaponState")
	bool bWeaponActive;

	/**
	 * Tag name for looking up the mesh component.
	 * 查找网格组件的标签名称。
	 */
	UPROPERTY(EditDefaultsOnly, Category="WeaponSetting")
	FName WeaponMeshTagName{TEXT("WeaponMesh")};

private:
	/**
	 * Cached primitive component for the weapon.
	 * 武器的缓存原始组件。
	 */
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> CachedPrimitiveComponent;
};