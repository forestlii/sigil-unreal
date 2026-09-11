// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/SigilAbilitySourceInterface.h"
#include "Abilities/SigilGameplayAbility.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffect.h"
#include "NativeGameplayTags.h"
#include "SigilAbilitySystemComponent.h"
#include "SigilGasTestTypes.generated.h"

namespace SigilGasTestTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SharedCooldown)
}

/**
 * Minimal avatar/owner actor carrying a Sigil ability system component for automation tests.
 * 自动化测试用的最小化身/拥有者 Actor，自带 Sigil 技能系统组件。
 */
UCLASS(Transient, NotPlaceable)
class ASigilGasTestAbilityActor final : public AActor
{
	GENERATED_BODY()

public:
	ASigilGasTestAbilityActor();

	USigilAbilitySystemComponent* GetAbilitySystem() const { return AbilitySystem; }

private:
	UPROPERTY()
	TObjectPtr<USigilAbilitySystemComponent> AbilitySystem;
};

/**
 * Ability that commits and (optionally) ends immediately, counting activations on the instance.
 * 立即提交并（可选）立即结束的技能，在实例上计数激活次数。
 */
UCLASS(Transient)
class USigilGasTestAbility : public USigilGameplayAbility
{
	GENERATED_BODY()

public:
	USigilGasTestAbility();

	void SetRequireSourceObjectActive(bool bInRequire) { bRequireSourceObjectActive = bInRequire; }

	int32 ActivationCount = 0;
	bool bEndImmediately = true;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

/**
 * Concrete UObject without any Sigil interface (UObject itself is abstract and cannot be instantiated).
 * 不带任何 Sigil 接口的具体 UObject（UObject 本身是抽象类，不能直接实例化）。
 */
UCLASS(Transient)
class USigilGasTestPlainObject final : public UObject
{
	GENERATED_BODY()
};

/**
 * Server-only passive ability tagged Sigil.Ability.Trait.ActivationOnSpawn that stays active once activated.
 * 带 Sigil.Ability.Trait.ActivationOnSpawn 标签、激活后保持激活的仅服务器被动技能。
 */
UCLASS(Transient)
class USigilGasTestPassiveAbility final : public USigilGasTestAbility
{
	GENERATED_BODY()

public:
	USigilGasTestPassiveAbility();
};

/**
 * Shared cooldown effect whose duration is a SetByCaller on Sigil.SetByCaller.CooldownDuration and which grants no tags itself.
 * 时长为 SetByCaller（Sigil.SetByCaller.CooldownDuration）、自身不授予任何标签的共享冷却效果。
 */
UCLASS(Transient)
class USigilGasTestSharedCooldownEffect final : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USigilGasTestSharedCooldownEffect();
};

/**
 * Cooldown effect with a fixed 2 second duration (engine path, no SetByCaller).
 * 固定 2 秒时长的冷却效果（引擎路径，无 SetByCaller）。
 */
UCLASS(Transient)
class USigilGasTestFixedCooldownEffect final : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USigilGasTestFixedCooldownEffect();
};

/**
 * Ability using the shared cooldown effect with its own cooldown tag and a 3 second SetByCaller duration.
 * 使用共享冷却效果、带自有冷却标签和 3 秒 SetByCaller 时长的技能。
 */
UCLASS(Transient)
class USigilGasTestSharedCooldownAbility final : public USigilGasTestAbility
{
	GENERATED_BODY()

public:
	USigilGasTestSharedCooldownAbility();
};

/**
 * Ability using the fixed-duration cooldown effect without any Sigil cooldown configuration (engine path).
 * 使用固定时长冷却效果、不配置任何 Sigil 冷却字段的技能（引擎路径）。
 */
UCLASS(Transient)
class USigilGasTestFixedCooldownAbility final : public USigilGasTestAbility
{
	GENERATED_BODY()

public:
	USigilGasTestFixedCooldownAbility();
};

/**
 * Player controller with a scripted view point, so target-actor aiming can be tested without a camera pipeline.
 * 视点可脚本化的玩家控制器，让目标 Actor 的瞄准测试不依赖相机管线。
 */
UCLASS(Transient, NotPlaceable)
class ASigilGasTestPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	FVector ScriptedViewLocation = FVector::ZeroVector;
	FRotator ScriptedViewRotation = FRotator::ZeroRotator;

	virtual void GetPlayerViewPoint(FVector& OutLocation, FRotator& OutRotation) const override
	{
		OutLocation = ScriptedViewLocation;
		OutRotation = ScriptedViewRotation;
	}
};

/**
 * Records cooldown / stack async-task broadcasts for assertions.
 * 记录冷却 / 堆叠异步任务的广播供断言。
 */
UCLASS(Transient)
class USigilGasTestAsyncListener final : public UObject
{
	GENERATED_BODY()

public:
	int32 CooldownBeginCount = 0;
	int32 CooldownEndCount = 0;
	int32 StackChangeCount = 0;
	FGameplayTag LastCooldownTag;
	float LastTimeRemaining = 0.f;
	float LastDuration = 0.f;
	FGameplayTag LastStackTag;
	int32 LastNewStackCount = -1;
	int32 LastOldStackCount = -1;

	UFUNCTION()
	void HandleCooldownBegin(FGameplayTag CooldownTag, float TimeRemaining, float Duration)
	{
		++CooldownBeginCount;
		LastCooldownTag = CooldownTag;
		LastTimeRemaining = TimeRemaining;
		LastDuration = Duration;
	}

	UFUNCTION()
	void HandleCooldownEnd(FGameplayTag CooldownTag, float TimeRemaining, float Duration)
	{
		++CooldownEndCount;
		LastCooldownTag = CooldownTag;
	}

	UFUNCTION()
	void HandleStackChanged(FGameplayTag EffectGameplayTag, FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 OldStackCount)
	{
		++StackChangeCount;
		LastStackTag = EffectGameplayTag;
		LastNewStackCount = NewStackCount;
		LastOldStackCount = OldStackCount;
	}
};

/**
 * Source object whose active state can be toggled by the test.
 * 测试可切换激活态的来源对象。
 */
UCLASS(Transient)
class USigilGasTestSourceObject final : public UObject, public ISigilAbilitySourceInterface
{
	GENERATED_BODY()

public:
	bool bActive = false;

	virtual bool IsAbilitySourceActive_Implementation() const override { return bActive; }
};
