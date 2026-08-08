// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilInputTypes.h"
#include "InputAction.h"
#include "UObject/Object.h"
#include "SigilInputChecker.generated.h"

class USigilInputSystemComponent;

/**
 * Base class for input validation, inheritable via Blueprint or C++.
 * 输入验证的基类，可通过蓝图或C++继承。
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, CollapseCategories, Const)
class SIGILINPUT_API USigilInputChecker : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Checks if an input is valid.
	 * 检查输入是否有效。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input is valid, false otherwise. 如果输入有效则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	bool CheckInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, UPARAM(meta = (Categories="InputTag,Sigil.Input.InputTag")) FGameplayTag InputTag,
	                ETriggerEvent TriggerEvent) const;

protected:
	/**
	 * Blueprint-implementable input validation logic.
	 * 可通过蓝图实现的输入验证逻辑。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input is valid, false otherwise. 如果输入有效则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIPS|Input", meta=(DisplayName="DoCheckInput"))
	bool DoCheckInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const;
};

/**
 * Input checker for tag-based relationships.
 * 基于标签关系的输入检查器。
 */
UCLASS()
class SIGILINPUT_API USigilInputChecker_TagRelationship : public USigilInputChecker
{
	GENERATED_BODY()

protected:
	/**
	 * List of input tag relationships for validation.
	 * 用于验证的输入标签关系列表。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input", meta=(TitleProperty="EditorFriendlyName", ShowOnlyInnerProperties))
	TArray<FSigilInputTagRelationship> InputTagRelationships;

	/**
	 * Gets the actor's tags for validation.
	 * 获取用于验证的演员标签。
	 * @param IC The input system component. 输入系统组件。
	 * @return The actor's gameplay tag container. 演员的游戏标签容器。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input", BlueprintNativeEvent)
	FGameplayTagContainer GetActorTags(USigilInputSystemComponent* IC) const;
	virtual FGameplayTagContainer GetActorTags_Implementation(USigilInputSystemComponent* IC) const;

	/**
	 * Implementation of input validation logic.
	 * 输入验证逻辑的实现。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input is valid, false otherwise. 如果输入有效则返回true，否则返回false。
	 */
	virtual bool DoCheckInput_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag,
	                                         const ETriggerEvent& TriggerEvent) const override;

#if WITH_EDITOR
	/**
	 * Called before saving the object.
	 * 在保存对象之前调用。
	 * @param SaveContext The save context. 保存上下文。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};