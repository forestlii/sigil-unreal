// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GCS_CombatStructLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MINOR_VERSION < 5
#include "InstancedStruct.h"
#else
#include "StructUtils/InstancedStruct.h"
#endif
#include "UObject/Object.h"
#include "GCS_BulletDefinition.generated.h"

class UNiagaraSystem;
class AGCS_BulletInstance;

/**
 * Data structure defining bullet properties and behavior.
 * 定义子弹属性和行为的数据结构。
 */
USTRUCT(BlueprintType, meta=(DisplayName="GCS Bullet Definition"))
struct GENERICCOMBATSYSTEM_API FGCS_BulletDefinition : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * The bullet actor class.
	 * 子弹Actor类。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common", meta=(AllowAbstract="false"))
	TSoftClassPtr<AGCS_BulletInstance> BulletActorClass;

	/**
	 * Duration for which the bullet exists (-1 for infinite).
	 * 子弹存在的持续时间（-1表示无限）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Common")
	float Duration{3.0};

	/**
	 * Number of bullets fired at once.
	 * 一次性发射的子弹数量。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Launch Configuration", meta=(ClampMin=1, UIMin=1))
	int32 BulletCount{1};

	/**
	 * Yaw angle for bullet launch.
	 * 子弹发射的水平角。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Launch Configuration")
	float LaunchAngle{0.0f};

	/**
	 * Yaw angle interval between bullets.
	 * 子弹之间的水平角间隔。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Launch Configuration")
	float LaunchAngleInterval{10.0f};

	/**
	 * Pitch angle for bullet launch.
	 * 子弹发射的仰角。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Launch Configuration")
	float LaunchElevationAngle{0.0f};

	/**
	 * Distance at which bullet attenuation begins.
	 * 子弹开始衰减的距离。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(Units="cm"))
	float AttenuationRange{800.0f};

	/**
	 * Gravity scale within the attenuation range.
	 * 衰减范围内的重力系数。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float GravityScaleInRange{1.0f};

	/**
	 * Gravity scale outside the attenuation range.
	 * 衰减范围外的重力系数。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float GravityScaleOutRage{1.0f};

	/**
	 * Initial hit radius for the bullet.
	 * 子弹的初始命中半径。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(Units="cm"))
	float InitialHitRadius{20.0f};

	/**
	 * Final hit radius for the bullet (-1 to use initial radius).
	 * 子弹的最终命中半径（-1使用初始半径）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(Units="cm"))
	float FinalHitRadius{-1.0f};

	/**
	 * Initial speed of the bullet.
	 * 子弹的初始速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(Units="cm"))
	float InitialSpeed{1500.0f};

	/**
	 * Minimum speed of the bullet.
	 * 子弹的最小速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(Units="cm"))
	float MinSpeed{1500.0f};

	/**
	 * Maximum speed of the bullet.
	 * 子弹的最大速度。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(Units="cm"))
	float MaxSpeed{1500.0f};

	/**
	 * Handle to the attack definition for the bullet.
	 * 子弹的攻击定义句柄。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack", meta=(RowType="/Script/GenericCombatSystem.GCS_AttackDefinition"))
	FDataTableRowHandle AttackDefinition;

	/**
	 * Trace definitions for hit detection.
	 * 用于命中检测的碰撞检测定义。
	 * @note Overrides trace definitions in the bullet instance class.
	 * @注意 覆盖子弹实例类中的碰撞检测定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	TArray<FGCS_CollisionTraceDefinition> TraceDefinitions;

	/**
	 * Visual effect for the bullet projectile (Niagara).
	 * 子弹的视觉效果（Niagara）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TSoftObjectPtr<UNiagaraSystem> ProjectileFX;

	/**
	 * Visual effect for the bullet projectile (Cascade).
	 * 子弹的视觉效果（Cascade）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TSoftObjectPtr<UParticleSystem> ProjectileFX_Cascade;

	/**
	 * Visual effect for bullet impact (Niagara).
	 * 子弹命中的视觉效果（Niagara）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TSoftObjectPtr<UNiagaraSystem> ImpactFX;

	/**
	 * Visual effect for bullet impact (Cascade).
	 * 子弹命中的视觉效果（Cascade）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TSoftObjectPtr<UParticleSystem> ImpactFX_Cascade;

	/**
	 * Sound effect for bullet impact.
	 * 子弹命中的音效。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX")
	TSoftObjectPtr<USoundBase> ImpactSFX;

	/**
	 * Sound effect attached to the bullet projectile.
	 * 附着在子弹上的音效。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX")
	TSoftObjectPtr<USoundBase> ProjectileSFX;

	/**
	 * Sound effect played once when the bullet spawns.
	 * 子弹生成时播放一次的音效。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX")
	TSoftObjectPtr<USoundBase> SpawnSFX;

	/**
	 * Whether the bullet penetrates characters/pawns.
	 * 子弹是否穿透角色/Pawn。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration")
	bool bPenetrateCharacter{false};

	/**
	 * Whether the bullet penetrates map geometry.
	 * 子弹是否穿透地图几何体。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration")
	bool bPenetrateMap{false};

	/**
	 * Handle to the bullet definition to spawn on hit/expiration.
	 * 命中或失效时生成的子弹定义句柄。
	 * @note Cannot be the same as this bullet.
	 * @注意 不能与此子弹相同。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit Configuration", meta=(RowType="/Script/GenericCombatSystem.GCS_BulletDefinition"))
	FDataTableRowHandle HitBulletDefinition;

	/**
	 * Condition for launching bullet chains.
	 * 子弹链的发射条件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit Configuration", meta=(Categories="GGF.Combat.Bullet.LaunchCond"))
	FGameplayTag LaunchCondition{FGameplayTag::EmptyTag};

	/**
	 * Custom user settings for extending the bullet definition.
	 * 扩展子弹定义的自定义用户设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extension", meta=(ForceInlineRow, BaseStruct = "/Script/GenericCombatSystem.GCS_UserSetting"))
	TMap<FGameplayTag, FInstancedStruct> UserSettings;
};
