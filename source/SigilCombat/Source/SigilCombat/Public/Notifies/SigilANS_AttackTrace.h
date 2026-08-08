// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SigilANS_AttackTrace.generated.h"

class USigilAttackRequest_Melee;

/**
 * Animation notify state for melee attack tracing.
 * 近战攻击追踪的动画通知状态。
 */
UCLASS(BlueprintType, Blueprintable, HideDropdown)
class SIGILCOMBAT_API USigilANS_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	USigilANS_AttackTrace(const FObjectInitializer& ObjectInitializer);

	/**
	 * Called after properties are initialized.
	 * 属性初始化后调用。
	 */
	virtual void PostInitProperties() override;

	/**
	 * The melee attack request instance.
	 * 近战攻击请求实例。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category=Parameters)
	TObjectPtr<USigilAttackRequest_Melee> AttackRequest;

#if WITH_EDITORONLY_DATA
	/**
	 * Called before saving to validate data.
	 * 保存前调用以验证数据。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};