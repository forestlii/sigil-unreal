// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilBulletDefinition.h"
#include "SigilEffectCauserInterface.h"
#include "GameFramework/Actor.h"
#include "SigilBulletInstance.generated.h"

class USigilAttackRequest_Bullet;
class USigilBulletSubsystem;
class UProjectileMovementComponent;

/**
 * Base class for bullet instances.
 * 子弹实例的基类。
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class SIGILCOMBAT_API ASigilBulletInstance : public AActor, public ISigilEffectCauserInterface
{
	GENERATED_BODY()

	friend USigilBulletSubsystem;

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	ASigilBulletInstance();

	/**
	 * Gets lifetime replicated properties.
	 * 获取生命周期复制属性。
	 * @param OutLifetimeProps The lifetime properties. 生命周期属性。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Gets the projectile movement component.
	 * 获取子弹的运动组件。
	 * @return The projectile movement component. 运动组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Bullet")
	UProjectileMovementComponent* GetProjectileMovementComponent() const;

	/**
	 * Sets the bullet definition handle.
	 * 设置子弹定义句柄。
	 * @param NewHandle The new definition handle. 新定义句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Bullet")
	void SetDefinitionHandle(UPARAM(meta=(RowType="/Script/SigilCombat.SigilBulletDefinition")) FDataTableRowHandle NewHandle);

	/**
	 * Sets the bullet's unique ID.
	 * 设置子弹的唯一ID。
	 * @param NewId The new ID. 新ID。
	 */
	void SetBulletId(const FGuid& NewId);

	/**
	 * Gets the bullet's unique ID.
	 * 获取子弹的唯一ID。
	 * @return The bullet ID. 子弹ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Bullet")
	FGuid GetBulletId() const;

	/**
	 * Sets the parent bullet ID for bullet chains.
	 * 设置子弹链的父子弹ID。
	 * @param NewParentId The parent bullet ID. 父子弹ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Bullet")
	void SetParentBulletId(FGuid NewParentId);

	/**
	 * Gets the parent bullet ID.
	 * 获取父子弹ID。
	 * @return The parent bullet ID. 父子弹ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Bullet")
	FGuid GetParentBulletId() const;

	/**
	 * Launches the bullet.
	 * 发射子弹。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Bullet")
	void LaunchBullet();

	/**
	 * Sets the hit result for the bullet.
	 * 设置子弹的命中结果。
	 * @param NewHitResult The hit result. 命中结果。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Bullet", meta=(DisplayName="Set Bullet HitResult"))
	void SetHitResult(const FHitResult& NewHitResult);

	/**
	 * Gets the latest hit result.
	 * 获取最新的命中结果。
	 * @return The hit result. 命中结果。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Bullet", meta=(DisplayName="Get Bullet HitResult"))
	const FHitResult& GetHitResult() const;

	/**
	 * Checks if the bullet has gameplay authority.
	 * 检查子弹是否具有游戏权限。
	 * @return True if the bullet has authority. 如果子弹具有权限返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Bullet")
	bool HasGameplayAuthority() const;

	/**
	 * Called after network initialization.
	 * 网络初始化后调用。
	 */
	virtual void PostNetInit() override;

	/**
	 * Called after receiving network data.
	 * 接收网络数据后调用。
	 */
	virtual void PostNetReceive() override;

	/**
	 * Handles locally predicted bullet instances.
	 * 处理本地预测的子弹实例。
	 * @param PredictedBullet The predicted bullet instance. 预测的子弹实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Bullet")
	void FoundLocalPredictedBullet(ASigilBulletInstance* PredictedBullet);

	// Effect causer interface
	/**
	 * Gets the gameplay effect spec handle.
	 * 获取游戏效果规格句柄。
	 * @param OutHandle The effect spec handle (output). 效果规格句柄（输出）。
	 * @return True if successful. 如果成功返回true。
	 */
	virtual bool GetEffectSpecHandle_Implementation(FGameplayEffectSpecHandle& OutHandle) override;

	/**
	 * Gets the gameplay effect container.
	 * 获取游戏效果容器。
	 * @return The effect container. 效果容器。
	 */
	virtual FSigilGameplayEffectContainer GetEffectContainer_Implementation() const override;

	/**
	 * Gets the effect container level override.
	 * 获取效果容器级别覆盖。
	 * @return The level override. 级别覆盖。
	 */
	virtual int32 GetEffectContainerLevelOverride_Implementation() const override;

	/**
	 * Sets the effect container spec.
	 * 设置效果容器规格。
	 * @param InEffectContainerSpec The effect container spec. 效果容器规格。
	 */
	virtual void SetEffectContainerSpec_Implementation(const FSigilGameplayEffectContainerSpec& InEffectContainerSpec) override;

	/**
	 * Gets the effect container spec.
	 * 获取效果容器规格。
	 * @return The effect container spec. 效果容器规格。
	 */
	virtual FSigilGameplayEffectContainerSpec GetEffectContainerSpec_Implementation() const override;

	/**
	 * Gets the effect class.
	 * 获取效果类。
	 * @return The effect class. 效果类。
	 */
	virtual TSubclassOf<UGameplayEffect> GetEffectClass_Implementation() const override;

	/**
	 * Gets the effect level.
	 * 获取效果级别。
	 * @return The effect level. 效果级别。
	 */
	virtual int32 GetEffectLevel_Implementation() const override;

	/**
	 * Sets the effect spec.
	 * 设置效果规格。
	 * @param InEffectSpec The effect spec. 效果规格。
	 */
	virtual void SetEffectSpec_Implementation(FGameplayEffectSpecHandle& InEffectSpec) override;

	/**
	 * Gets the bullet's shape component.
	 * 获取子弹的形状组件。
	 * @return The shape component. 形状组件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GCS|Bullet")
	UShapeComponent* GetBulletShape() const;
	virtual UShapeComponent* GetBulletShape_Implementation() const;

protected:
	/**
	 * Called when the game starts or when spawned.
	 * 游戏开始或生成时调用。
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when the game ends or the bullet is destroyed.
	 * 游戏结束或子弹销毁时调用。
	 * @param EndPlayReason The reason for ending. 结束原因。
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Called when the bullet is taken from the pool with a valid definition.
	 * 子弹从池中取出且具有有效定义时调用。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Bullet")
	void OnBulletBeginPlay();

	/**
	 * Called when the bullet is returned to the pool and deactivated.
	 * 子弹返回池中并停用时调用。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Bullet")
	void OnBulletEndPlay();

	/**
	 * Records the initial location and rotation of the bullet.
	 * 记录子弹的初始位置和旋转。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Bullet")
	void SetupInitialLocationAndRotation();

	/**
	 * Refreshes the bullet's travel states.
	 * 刷新子弹的移动状态。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Bullet")
	virtual void RefreshTravelStates();

	/**
	 * Checks if the hit result should be penetrated.
	 * 检查命中结果是否应穿透。
	 * @param InHitResult The hit result. 命中结果。
	 * @return True if the hit should be penetrated. 如果命中应穿透返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Bullet")
	virtual bool ShouldPenetrateHitResult(const FHitResult& InHitResult) const;

	/**
	 * Checks if a bullet chain should be generated.
	 * 检查是否应生成子弹链。
	 * @return True if a bullet chain should be generated. 如果应生成子弹链返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="GCS|Bullet")
	bool ShouldGenerateBullet();

	/**
	 * Handles bullet chain logic on hit.
	 * 处理命中时的子弹链逻辑。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Bullet")
	void HandleBulletHitChains();

	/**
	 * Applies gameplay effects to the hit result.
	 * 对命中结果应用游戏效果。
	 * @param HitResult The hit result. 命中结果。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GCS|Bullet")
	void ApplyGameplayEffects(FHitResult HitResult);

	/**
	 * Called when the bullet ID is replicated.
	 * 子弹ID复制时调用。
	 * @param Prev The previous bullet ID. 之前的子弹ID。
	 */
	UFUNCTION()
	virtual void OnRep_BulletId(FGuid Prev);

	/**
	 * Called when the bullet definition is replicated.
	 * 子弹定义复制时调用。
	 */
	UFUNCTION()
	void OnRep_BulletDefinition();

	/**
	 * Unique ID of the bullet.
	 * 子弹的唯一ID。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_BulletId, Category="GCS|BulletState")
	FGuid BulletId;

	/**
	 * Whether the bullet is locally predicted.
	 * 子弹是否为本地预测。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GCS|BulletState")
	bool bIsLocalPredicting{false};

	/**
	 * Whether the bullet was spawned on the server.
	 * 子弹是否在服务器上生成。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GCS|BulletState")
	bool bServerInitiated{false};

	/**
	 * Parent bullet ID for bullet chains.
	 * 子弹链的父子弹ID。
	 */
	UPROPERTY(VisibleAnywhere, Category="GCS|BulletState")
	FGuid ParentBulletId;

	/**
	 * The associated attack request.
	 * 关联的攻击请求。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GCS|BulletState")
	TObjectPtr<const USigilAttackRequest_Bullet> Request;

	/**
	 * Handle to the bullet definition.
	 * 子弹定义的句柄。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS|Bullet", ReplicatedUsing=OnRep_BulletDefinition, meta=(ExposeOnSpawn))
	FDataTableRowHandle DefinitionHandle;

	/**
	 * Loaded bullet definition.
	 * 加载的子弹定义。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GCS|BulletState")
	FSigilBulletDefinition Definition;

	/**
	 * The projectile movement component.
	 * 子弹的运动组件。
	 */
	UPROPERTY(VisibleAnywhere, Category="GCS|Bullet")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/**
	 * The gameplay effect spec carried by the bullet.
	 * 子弹携带的游戏效果规格。
	 */
	UPROPERTY(VisibleAnywhere, Category="GCS|BulletState")
	FGameplayEffectSpecHandle EffectSpecHandle;

	/**
	 * The gameplay effect container spec carried by the bullet.
	 * 子弹携带的游戏效果容器规格。
	 */
	UPROPERTY(VisibleAnywhere, Category = "GCS|BulletState")
	FSigilGameplayEffectContainerSpec EffectContainerSpec;

	/**
	 * Index for bullet chains.
	 * 子弹链的索引。
	 */
	UPROPERTY()
	int32 ChainIndex{0};

	/**
	 * Initial location of the bullet.
	 * 子弹的初始位置。
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="GCS|BulletState")
	FVector InitialActorLocation;

	/**
	 * Initial rotation of the bullet.
	 * 子弹的初始旋转。
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="GCS|BulletState")
	FRotator InitialActorRotation;

	/**
	 * The latest hit result.
	 * 最新的命中结果。
	 */
	UPROPERTY(VisibleAnywhere, Category="GCS|BulletState")
	FHitResult LastHitResult;

	/**
	 * Distance traveled by the bullet.
	 * 子弹的移动距离。
	 */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="GCS|BulletState")
	double TraveledDistance{0.0f};

public:
	/**
	 * Called every frame.
	 * 每帧调用。
	 * @param DeltaTime Time since last frame. 上一帧以来的时间。
	 */
	virtual void Tick(float DeltaTime) override;
};