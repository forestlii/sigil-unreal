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
	 * Server-side distance limit (in world units) between an instigator and the target of a predictable montage request.
	 * 0 disables the check. Used by USigilCombatSystemComponent::CanPlayMontageOnTarget's default implementation.
	 * 服务器侧校验：可预测蒙太奇请求的发起者与目标之间允许的最大距离（世界单位）。0 表示不检查。
	 * 由 USigilCombatSystemComponent::CanPlayMontageOnTarget 的默认实现使用。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Networking", meta=(ClampMin=0, Units="cm"))
	float MaxPredictableMontageTargetDistance{0.0f};

	/**
	 * Disables affiliation checks for debugging (allows cross-team damage).
	 * 禁用归属检查以进行调试（允许跨队伍伤害）。
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, NoClear, Category="Debug")
	bool bDisableAffiliationCheck{false};
};