// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GCS_ANS_BulletTrace.generated.h"

class UGCS_AttackRequest_Bullet;
class UTargetingPreset;

/**
 * Animation notify state for bullet attack tracing.
 * 子弹攻击追踪的动画通知状态。
 */
UCLASS(BlueprintType, Blueprintable, HideDropdown)
class GENERICCOMBATSYSTEM_API UGCS_ANS_BulletTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	UGCS_ANS_BulletTrace(const FObjectInitializer& ObjectInitializer);

	/**
	 * The bullet attack request instance.
	 * 子弹攻击请求实例。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category=Parameters)
	TObjectPtr<UGCS_AttackRequest_Bullet> AttackRequest;

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