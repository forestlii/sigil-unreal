// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Abilities/SigilGameplayAbility_FireCadence.h"

#include "Engine/World.h"
#include "Equipping/SigilWeaponEquipmentInstance.h"
#include "SigilAbilitySystemComponent.h"
#include "Utilities/SigilAbilitySystemFunctionLibrary.h"

USigilGameplayAbility_FireCadence::USigilGameplayAbility_FireCadence()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	bRequireSourceObjectActive = true;
}

bool USigilGameplayAbility_FireCadence::CanActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const float Interval = 60.0f / RoundsPerMinute;
	if (!FMath::IsFinite(RoundsPerMinute) || RoundsPerMinute <= 0.0f || !FMath::IsFinite(Interval) || Interval <= 0.0f
		|| (FireMode != ESigilFireMode::SemiAuto && FireMode != ESigilFireMode::FullAuto && FireMode != ESigilFireMode::Burst)
		|| (FireMode == ESigilFireMode::Burst && BurstCount <= 0))
	{
		return false;
	}

	const USigilWeaponEquipmentInstance* Weapon = Cast<USigilWeaponEquipmentInstance>(GetSourceObject(Handle, ActorInfo));
	return IsValid(Weapon) && ISigilAbilitySourceInterface::Execute_IsAbilitySourceActive(Weapon);
}

void USigilGameplayAbility_FireCadence::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	++ActivationSerial;
	RoundsFired = 0;
	ActiveFireMode = FireMode;
	ActiveBurstCount = BurstCount;
	SourceWeapon = Cast<USigilWeaponEquipmentInstance>(GetSourceObject(Handle, ActorInfo));
	TimerWorld = GetWorld();

	if (!SourceWeapon.IsValid() || !TimerWorld.IsValid())
	{
		StopCadence(true);
		return;
	}

	SourceWeapon->OnActiveStateChangedEvent.AddUniqueDynamic(this, &ThisClass::OnWeaponActiveStateChanged);
	// 先登记 Timer，再发首发；首发同步取消或重启时，不会在回调返回后重建旧 Timer。
	if (ActiveFireMode != ESigilFireMode::SemiAuto && !(ActiveFireMode == ESigilFireMode::Burst && ActiveBurstCount == 1))
	{
		TimerWorld->GetTimerManager().SetTimer(FireTimer, this, &ThisClass::FireNextRound, 60.0f / RoundsPerMinute, true);
	}
	FireNextRound();
}

void USigilGameplayAbility_FireCadence::FireNextRound()
{
	if (!IsActive())
	{
		return;
	}
	if (!SourceWeapon.IsValid() || !ISigilAbilitySourceInterface::Execute_IsAbilitySourceActive(SourceWeapon.Get()))
	{
		StopCadence(true);
		return;
	}

	const uint64 ThisActivation = ActivationSerial;
	const bool bFired = TryFireOnce();
	// 扩展钩子或单发可能同步结束并重新激活同一实例；旧调用不能修改新一轮状态。
	if (!IsActive() || ActivationSerial != ThisActivation)
	{
		return;
	}
	if (!bFired)
	{
		StopCadence(false);
		return;
	}
	if (ActiveFireMode == ESigilFireMode::SemiAuto
		|| (ActiveFireMode == ESigilFireMode::Burst && ++RoundsFired >= ActiveBurstCount))
	{
		StopCadence(false);
	}
}

bool USigilGameplayAbility_FireCadence::TryFireOnce_Implementation()
{
	USigilAbilitySystemComponent* ASC = Cast<USigilAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!ASC || !SourceWeapon.IsValid() || !SingleShotAbilityClass)
	{
		return false;
	}

	const FGameplayAbilitySpecHandle ShotHandle = USigilAbilitySystemFunctionLibrary::FindAbilitySpecHandleForClass(
		ASC, SingleShotAbilityClass, SourceWeapon.Get());
	const FGameplayAbilitySpec* ShotSpec = ShotHandle.IsValid() ? ASC->FindAbilitySpecFromHandle(ShotHandle) : nullptr;
	if (!ShotSpec || ShotHandle == GetCurrentAbilitySpecHandle() || ShotSpec->PendingRemove || ShotSpec->IsActive())
	{
		return false;
	}

	// 单发自行 Commit 和 End，避免批处理的强制 End 路径误结束未成功激活的能力。
	return ASC->BatchRPCTryActivateAbility(ShotHandle, false);
}

void USigilGameplayAbility_FireCadence::InputReleased(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayAbilityActivationInfo ActivationInfo)
{
	StopCadence(false);
}

void USigilGameplayAbility_FireCadence::OnWeaponActiveStateChanged(bool bNewActiveState)
{
	if (!bNewActiveState)
	{
		StopCadence(true);
	}
}

void USigilGameplayAbility_FireCadence::StopCadence(bool bWasCancelled)
{
	if (IsActive())
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, bWasCancelled);
	}
}

void USigilGameplayAbility_FireCadence::EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo))
	{
		return;
	}
	if (ScopeLockCount > 0)
	{
		const uint64 EndingActivation = ActivationSerial;
		WaitingToExecute.Add(FPostLockDelegate::CreateWeakLambda(this,
			[this, EndingActivation, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled]()
			{
				// 锁内可重复请求结束；前一条结束通知重启后，旧排队项不能结束新一轮。
				if (ActivationSerial == EndingActivation)
				{
					EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
				}
			}));
		return;
	}

	++ActivationSerial;
	if (TimerWorld.IsValid())
	{
		TimerWorld->GetTimerManager().ClearTimer(FireTimer);
	}
	if (SourceWeapon.IsValid())
	{
		SourceWeapon->OnActiveStateChangedEvent.RemoveDynamic(this, &ThisClass::OnWeaponActiveStateChanged);
	}
	FireTimer.Invalidate();
	TimerWorld.Reset();
	SourceWeapon.Reset();
	RoundsFired = 0;
	// 清理在 Super 之前完成，防止结束通知重入后误清理新一轮激活。
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
