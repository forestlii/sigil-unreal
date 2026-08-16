// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SigilCombatSystemSettings.generated.h"

/**
 * Settings for the combat system.
 * 战斗系统的设置。
 */
UCLASS(Config=Game, DefaultConfig)
class SIGILCOMBAT_API USigilCombatSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Gets the combat system settings instance.
	 * 获取战斗系统设置实例。
	 * @return The combat system settings. 战斗系统设置。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCS")
	static const USigilCombatSystemSettings* Get();

	/**
	 * Tag name for querying the main skeletal mesh component.
	 * 查询主要骨骼网格组件的标签名称。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, NoClear, Category="Common", meta=(DisplayName="Main Mesh Lookup Tag Name"))
	FName CharacterMeshLookupTag{TEXT("Main")};

	/**
	 * Opt-in for cross-target predictable montages: the maximum server-side distance (world units) between an
	 * instigator and a *different* target. 0 (default) means the default authorization rejects every target other
	 * than the instigator itself. Used by USigilCombatSystemComponent::CanPlayMontageOnTarget's default implementation.
	 * 跨目标可预测蒙太奇的开关：发起者与**其它**目标之间在服务器侧允许的最大距离（世界单位）。
	 * 0（默认）表示默认授权拒绝除自身外的所有目标。由 USigilCombatSystemComponent::CanPlayMontageOnTarget 的默认实现使用。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Networking", meta=(ClampMin=0, Units="cm"))
	float MaxPredictableMontageTargetDistance{0.0f};

	/**
	 * Lowest play rate a predictable montage request may carry (rejected below this).
	 * 可预测蒙太奇请求允许的最低播放倍率（低于此值拒绝）。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Networking", meta=(ClampMin=0.01))
	float MinPredictableMontagePlayRate{0.1f};

	/**
	 * Highest play rate a predictable montage request may carry (rejected above this).
	 * 可预测蒙太奇请求允许的最高播放倍率（高于此值拒绝）。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Networking", meta=(ClampMin=0.1))
	float MaxPredictableMontagePlayRate{4.0f};

	/**
	 * Highest root translation scale a predictable montage request may carry.
	 * 可预测蒙太奇请求允许的最大根运动平移缩放。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Networking", meta=(ClampMin=0))
	float MaxPredictableMontageRootTranslationScale{10.0f};

	/**
	 * Server-side cap on montage requests accepted per instigating component per second (0 = unlimited).
	 * 服务器侧每个发起组件每秒接受的蒙太奇请求上限（0 = 不限）。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Networking", meta=(ClampMin=0))
	int32 MaxPredictableMontageRequestsPerSecond{8};

	/**
	 * Disables affiliation checks for debugging (allows cross-team damage).
	 * 禁用归属检查以进行调试（允许跨队伍伤害）。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, NoClear, Category="Debug")
	bool bDisableAffiliationCheck{false};
};