// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SigilInputTypes.h"
#include "Engine/DataAsset.h"
#include "SigilInputConfig.generated.h"

class UInputAction;

/**
 * Configuration data asset for the input system component.
 * 输入系统组件的配置数据资产。
 */
UCLASS(Const)
class SIGILINPUT_API USigilInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Mapping of input tags to input action settings.
	 * 输入标签到输入动作设置的映射。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input", meta=(Categories="InputTag,Sigil.Input.InputTag", ForceInlineRow))
	TMap<FGameplayTag, FSigilInputActionSetting> InputActionMappings;

	/**
	 * List of defined input buffer windows.
	 * 定义的输入缓冲窗口列表。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input", meta=(DisplayName="Input Buffer Windows", TitleProperty="Tag", NoElementDuplicate))
	TArray<FSigilInputBufferWindow> InputBufferDefinitions;

protected:
#if WITH_EDITOR
	/**
	 * Called before saving the object.
	 * 在保存对象之前调用。
	 * @param SaveContext The save context. 保存上下文。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	/**
	 * Validates the data asset in the editor.
	 * 在编辑器中验证数据资产。
	 * @param Context The data validation context. 数据验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};