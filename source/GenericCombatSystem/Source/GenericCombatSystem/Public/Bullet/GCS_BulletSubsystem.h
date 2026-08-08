// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCS_BulletDefinition.h"
#include "Subsystems/WorldSubsystem.h"
#include "GCS_BulletSubsystem.generated.h"

class UGCS_AttackRequest_Bullet;
class AGCS_BulletInstance;

/**
 * Parameters for spawning bullets.
 * 子弹生成参数。
 */
USTRUCT(BlueprintType)
struct GENERICCOMBATSYSTEM_API FGCS_BulletSpawnParameters
{
	GENERATED_BODY()

public:
	/**
	 * The owner of the bullet.
	 * 子弹的拥有者。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	TObjectPtr<AActor> Owner{nullptr};

	/**
	 * Handle to the bullet definition.
	 * 子弹定义的句柄。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS", meta=(RowType="/Script/GenericCombatSystem.GCS_BulletDefinition"))
	FDataTableRowHandle DefinitionHandle;

	/**
	 * Transform for spawning the bullet.
	 * 子弹生成时的变换。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FTransform SpawnTransform{FTransform::Identity};

	/**
	 * The associated attack request.
	 * 关联的攻击请求。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	TObjectPtr<const UGCS_AttackRequest_Bullet> Request{nullptr};

	/**
	 * Whether the bullet is locally predicted.
	 * 子弹是否为本地预测。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	bool bIsLocalPredicting{false};

	/**
	 * IDs for locally predicted bullets.
	 * 本地预测子弹的ID。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS", meta=(DisplayName="Local Predicting Bullet Ids"))
	TArray<FGuid> OverrideBulletIds;

	/**
	 * ID of the parent bullet (for bullet chains).
	 * 父子弹的ID（用于子弹链）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GCS")
	FGuid ParentId;

	/**
	 * Returns a debug string representation.
	 * 返回调试字符串表示。
	 * @return The debug string. 调试字符串。
	 */
	FString ToDebugString() const;
};

/**
 * Subsystem for managing bullet spawning and lifecycle.
 * 管理子弹生成和生命周期的子系统。
 */
UCLASS()
class GENERICCOMBATSYSTEM_API UGCS_BulletSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Spawns bullets based on the provided parameters.
	 * 根据提供的参数生成子弹。
	 * @param SpawnParameters The spawn parameters. 生成参数。
	 * @return The spawned bullet instances. 生成的子弹实例。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Bullet")
	virtual TArray<AGCS_BulletInstance*> SpawnBullets(const FGCS_BulletSpawnParameters& SpawnParameters);

	/**
	 * Gets the IDs of the provided bullet instances.
	 * 获取提供的子弹实例的ID。
	 * @param Instances The bullet instances. 子弹实例。
	 * @return The IDs of the bullets. 子弹的ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS|Bullet")
	virtual TArray<FGuid> GetIdsFromBullets(TArray<AGCS_BulletInstance*> Instances);

	/**
	 * Gets or creates bullet instances based on parameters and definition.
	 * 根据参数和定义获取或创建子弹实例。
	 * @param SpawnParameters The spawn parameters. 生成参数。
	 * @param Definition The bullet definition. 子弹定义。
	 * @return The bullet instances. 子弹实例。
	 */
	TArray<AGCS_BulletInstance*> GetOrCreateBulletInstances(const FGCS_BulletSpawnParameters& SpawnParameters, const FGCS_BulletDefinition& Definition);

	/**
	 * Retrieves a bullet instance from the pool.
	 * 从池中获取子弹实例。
	 * @param BulletClass The class of the bullet. 子弹的类。
	 * @return The bullet instance. 子弹实例。
	 */
	AGCS_BulletInstance* TakeBulletFromPool(TSubclassOf<AGCS_BulletInstance> BulletClass);

	/**
	 * Destroys a bullet by its ID.
	 * 根据ID销毁子弹。
	 * @param BulledId The bullet ID. 子弹ID。
	 */
	UFUNCTION(BlueprintCallable, Category="GCS|Bullet")
	virtual void DestroyBullet(FGuid BulledId);

	/**
	 * Creates a single bullet instance.
	 * 创建单个子弹实例。
	 * @param SpawnParameters The spawn parameters. 生成参数。
	 * @param Definition The bullet definition. 子弹定义。
	 * @return The created bullet instance. 创建的子弹实例。
	 */
	AGCS_BulletInstance* CreateBulletInstance(const FGCS_BulletSpawnParameters& SpawnParameters, const FGCS_BulletDefinition& Definition);

	/**
	 * Loads a bullet definition from a handle.
	 * 从句柄加载子弹定义。
	 * @param Handle The bullet definition handle. 子弹定义句柄。
	 * @param OutDefinition The loaded definition (output). 加载的定义（输出）。
	 * @return True if the definition was loaded successfully. 如果定义加载成功返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GCS|Bullet", meta=(ExpandBoolAsExecs="ReturnValue"))
	virtual bool LoadBulletDefinition(const FDataTableRowHandle& Handle, FGCS_BulletDefinition& OutDefinition);

	/**
	 * Active bullet instances mapped by their IDs.
	 * 按ID映射的激活子弹实例。
	 */
	UPROPERTY()
	TMap<FGuid, TObjectPtr<AGCS_BulletInstance>> BulletInstances;

	/**
	 * Pool of bullet instances for reuse.
	 * 用于重用的子弹实例池。
	 */
	UPROPERTY()
	TArray<TObjectPtr<AGCS_BulletInstance>> BulletPools;
};