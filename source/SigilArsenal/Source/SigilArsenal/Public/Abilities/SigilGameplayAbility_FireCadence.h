// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Abilities/SigilGameplayAbility.h"
#include "TimerManager.h"
#include "SigilGameplayAbility_FireCadence.generated.h"

class USigilWeaponEquipmentInstance;

UENUM(BlueprintType)
enum class ESigilFireMode : uint8
{
	SemiAuto,
	FullAuto,
	Burst
};

/** 本地射击节奏；单发能力负责 Commit、扣弹与自身结束。首发立即触发。 */
UCLASS(Abstract, Blueprintable)
class SIGILARSENAL_API USigilGameplayAbility_FireCadence : public USigilGameplayAbility
{
	GENERATED_BODY()

public:
	USigilGameplayAbility_FireCadence();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arsenal|Cadence")
	ESigilFireMode FireMode = ESigilFireMode::SemiAuto;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arsenal|Cadence", meta = (ClampMin = "1"))
	float RoundsPerMinute = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arsenal|Cadence", meta = (ClampMin = "1"))
	int32 BurstCount = 3;

	/** 与本技能由同一装备授予、激活中自行 Commit 并 End 的单发能力。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arsenal|Cadence")
	TSubclassOf<USigilGameplayAbility> SingleShotAbilityClass;

	virtual bool CanActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void InputReleased(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 返回 false 立即停止节奏。true 只表示接受了单发激活请求，不代表命中或服务端确认。 */
	UFUNCTION(BlueprintNativeEvent, Category = "Arsenal|Cadence")
	bool TryFireOnce();
	virtual bool TryFireOnce_Implementation();

private:
	void FireNextRound();
	void StopCadence(bool bWasCancelled);

	UFUNCTION()
	void OnWeaponActiveStateChanged(bool bNewActiveState);

	FTimerHandle FireTimer;
	TWeakObjectPtr<UWorld> TimerWorld;
	TWeakObjectPtr<USigilWeaponEquipmentInstance> SourceWeapon;
	uint64 ActivationSerial = 0;
	int32 RoundsFired = 0;
	ESigilFireMode ActiveFireMode = ESigilFireMode::SemiAuto;
	int32 ActiveBurstCount = 0;
};
