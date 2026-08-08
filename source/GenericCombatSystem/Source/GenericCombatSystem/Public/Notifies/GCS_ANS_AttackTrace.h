// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GCS_ANS_AttackTrace.generated.h"

class UGCS_AttackRequest_Melee;

/**
 * Animation notify state for melee attack tracing.
 * 近战攻击追踪的动画通知状态。
 */
UCLASS(BlueprintType, Blueprintable, HideDropdown)
class GENERICCOMBATSYSTEM_API UGCS_ANS_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	UGCS_ANS_AttackTrace(const FObjectInitializer& ObjectInitializer);

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
	TObjectPtr<UGCS_AttackRequest_Melee> AttackRequest;

#if WITH_EDITORONLY_DATA
	/**
	 * Called before saving to validate data.
	 * 保存前调用以验证数据。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};