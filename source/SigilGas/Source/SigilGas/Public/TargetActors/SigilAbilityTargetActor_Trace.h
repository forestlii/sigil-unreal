// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "CollisionQueryParams.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SigilAbilityTargetActor_Trace.generated.h"

/**
 * 可重用、配置的目标捕获Actor。子类继承实现新的检测形状。
 * 与WaitTargetDataWithResuableActor配合使用。
 */
UCLASS()
class SIGILGAS_API ASigilAbilityTargetActor_Trace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ASigilAbilityTargetActor_Trace();

	// 基本瞄准扩散（角度）
	UPROPERTY(BlueprintReadWrite, Category = "GGA|TargetActor")
	float BaseSpread;

	// 瞄准扩散修改器
	UPROPERTY(BlueprintReadWrite, Category = "GGA|TargetActor")
	float AimingSpreadMod;

	// 连续瞄准: 扩散增量
	UPROPERTY(BlueprintReadWrite, Category = "GGA|TargetActor")
	float TargetingSpreadIncrement;

	// 连续瞄准: 最大增量
	UPROPERTY(BlueprintReadWrite, Category = "GGA|TargetActor")
	float TargetingSpreadMax;

	// 连续瞄准的当前扩散
	float CurrentTargetingSpread;

	/** 是否使用瞄准扩散，开启后，检测方向会产生随机扩散（轻微改变检测方向） */
	UPROPERTY(BlueprintReadWrite, Category = "GGA|TargetActor")
	bool bUseAimingSpreadMod;

	UPROPERTY(BlueprintReadWrite, Category = "GGA|TargetActor")
	FGameplayTag AimingTag;

	UPROPERTY(BlueprintReadWrite, Category = "GGA|TargetActor")
	FGameplayTag AimingRemovalTag;

	/** 最大范围 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	float MaxRange;

	/** 检测预设 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	FCollisionProfileName TraceProfile;

	/** 检测是否影响瞄准偏移(沿Y轴的旋转) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	bool bTraceAffectsAimPitch;

	/** 每次检测所返回的最大碰撞结果数量。0表示只返回检测终点。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	int32 MaxHitResultsPerTrace;

	/** 一次性检测的次数，单发射击武器（如来福枪）只会进行一次检测，而多发射击武器（如散弹枪）可以进行多次检测。
	 * 不可与PersistentHits配合使用。
	 * 会影响十字线Actor的数量
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	int32 NumberOfTraces;

	/**是否忽略阻挡Hits*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	bool bIgnoreBlockingHits;

	/**是否从玩家控制器的视角开始检测,否则从StartLocation开始检测*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	bool bTraceFromPlayerViewPoint;

	// HitResults will persist until Confirmation/Cancellation or until a new HitResult takes its place
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "GGA|TargetActor")
	bool bUsePersistentHitResults;

	/**重置扩散 */
	UFUNCTION(BlueprintCallable, Category = "GGA|TargetActor")
	virtual void ResetSpread();

	virtual float GetCurrentSpread() const;

	// 设置开始位置信息
	UFUNCTION(BlueprintCallable, Category = "GGA|TargetActor")
	void SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation);

	// 是否在服务端产生目标数据
	UFUNCTION(BlueprintCallable, Category = "GGA|TargetActor")
	virtual void SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer);

	// 设置当玩家确认目标后是否销毁此TargetingActor。
	UFUNCTION(BlueprintCallable, Category = "GGA|TargetActor")
	void SetDestroyOnConfirmation(bool bInDestroyOnConfirmation = false);

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;

	virtual void CancelTargeting() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	// Traces as normal, but will manually filter all hit actors
	virtual void LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World,
	                                 const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
	                                 const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	virtual void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params,
	                                     const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch = false);

	virtual bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter,
	                                         float AbilityRange, FVector& ClippedPosition);

	virtual void StopTargeting();

protected:
	// 检测终点, useful for debug drawing
	FVector CurrentTraceEnd;

	// 对准心Actor的引用
	TArray<TWeakObjectPtr<AGameplayAbilityWorldReticle>> ReticleActors;
	TArray<FHitResult> PersistentHitResults;

	TArray<FHitResult> CurrentHitResults;

	virtual FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<FHitResult>& HitResults) const;
	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor);


	//实际的检测，由子类覆写。
	virtual void DoTrace(TArray<FHitResult>& HitResults, const UWorld* World,
	                     const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End,
	                     FName ProfileName, const FCollisionQueryParams Params) PURE_VIRTUAL(AUETA_Trace, return;);

	virtual void ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType,
	                            float Duration = 2.0f) PURE_VIRTUAL(AUETA_Trace, return;);

	/** 生成准心Actor */
	virtual AGameplayAbilityWorldReticle* SpawnReticleActor(FVector Location, FRotator Rotation);

	/**销毁所有准心Actor */
	virtual void DestroyReticleActors();

public:
	/**获取当前的hitresult */
	UFUNCTION(BlueprintPure, Category = "GGA|TargetActor")
	void GetCurrentHitResult(TArray<FHitResult>& HitResults);
};
