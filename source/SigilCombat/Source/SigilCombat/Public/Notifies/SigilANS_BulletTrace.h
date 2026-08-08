// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SigilANS_BulletTrace.generated.h"

class USigilAttackRequest_Bullet;
class UTargetingPreset;

/**
 * Animation notify state for bullet attack tracing.
 * 子弹攻击追踪的动画通知状态。
 */
UCLASS(BlueprintType, Blueprintable, HideDropdown)
class SIGILCOMBAT_API USigilANS_BulletTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	USigilANS_BulletTrace(const FObjectInitializer& ObjectInitializer);

	/**
	 * The bullet attack request instance.
	 * 子弹攻击请求实例。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category=Parameters)
	TObjectPtr<USigilAttackRequest_Bullet> AttackRequest;

#if WITH_EDITOR
	/**
	 * Validates data for the notify.
	 * 验证通知数据。
	 * @param Context The validation context. 验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	/**
	 * Called before saving to validate data.
	 * 保存前调用以验证数据。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};