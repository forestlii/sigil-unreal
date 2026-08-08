// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "SigilInputProcessor.generated.h"

class USigilInputChecker;
struct FInputActionInstance;
enum class ETriggerEvent : uint8;
class USigilInputSystemComponent;

/**
 * Base class for processing input actions.
 * 处理输入动作的基类。
 */
UCLASS(EditInlineNew, DefaultToInstanced, CollapseCategories, Blueprintable, Const, HideDropdown)
class SIGILINPUT_API USigilInputProcessor : public UObject
{
	GENERATED_BODY()

public:
	USigilInputProcessor();

	/**
	 * Indicates if the processor supports networking.
	 * 指示处理器是否支持网络。
	 * @return True if networking is supported, false otherwise. 如果支持网络则返回true，否则返回false。
	 */
	virtual bool IsSupportedForNetworking() const override { return true; }

	/**
	 * Checks if the processor can handle the given input.
	 * 检查处理器是否可以处理给定的输入。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input can be handled, false otherwise. 如果可以处理输入则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input", meta=(AutoCreateRefTerm="ActionData"))
	bool CanHandleInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, UPARAM(meta = (Categories="InputTag,Sigil.Input.InputTag")) FGameplayTag InputTag,
	                    ETriggerEvent TriggerEvent) const;

	/**
	 * Handles the input action.
	 * 处理输入动作。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input", meta=(AutoCreateRefTerm="ActionData"))
	void HandleInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, UPARAM(meta = (Categories="InputTag,Sigil.Input.InputTag")) FGameplayTag InputTag,
	                 ETriggerEvent TriggerEvent) const;

	/**
	 * Tags that this processor responds to (if empty, responds to all inputs).
	 * 处理器响应的输入标签（如果为空，则响应所有输入）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputProcessor", meta=(Categories="InputTag,Sigil.Input.InputTag", DisplayPriority=0))
	FGameplayTagContainer InputTags;

	/**
	 * Trigger events that this processor responds to.
	 * 处理器响应的触发事件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InputProcessor", meta=(DisplayPriority=1))
	TArray<ETriggerEvent> TriggerEvents{ETriggerEvent::Started};

protected:
	/**
	 * Gets the input action value (deprecated).
	 * 获取输入动作值（已弃用）。
	 * @param ActionData The input action data. 输入动作数据。
	 * @return The input action value. 输入动作值。
	 */
	UFUNCTION(BlueprintPure, Category="InputProcessor", meta=(DeprecatedFunction, DeprecationMessage="Use GetInputActionValueOfInputTag"))
	FInputActionValue GetInputActionValue(const FInputActionInstance& ActionData) const;

	/**
	 * Blueprint-implementable check for input handling.
	 * 可通过蓝图实现的输入处理检查。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input can be handled, false otherwise. 如果可以处理输入则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="InputProcessor")
	bool CheckCanHandleInput(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const;

	/**
	 * Native implementation of input handling check.
	 * 输入处理检查的原生实现。
	 */
	virtual bool CheckCanHandleInput_Implementation(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent) const;

	/**
	 * Handles the triggered input event.
	 * 处理触发的输入事件。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="InputProcessor")
	void HandleInputTriggered(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const;

	/**
	 * Handles the started input event.
	 * 处理开始的输入事件。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="InputProcessor")
	void HandleInputStarted(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const;

	/**
	 * Handles the ongoing input event.
	 * 处理持续的输入事件。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="InputProcessor")
	void HandleInputOngoing(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const;

	/**
	 * Handles the canceled input event.
	 * 处理取消的输入事件。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="InputProcessor")
	void HandleInputCanceled(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const;

	/**
	 * Handles the completed input event.
	 * 处理完成的输入事件。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="InputProcessor")
	void HandleInputCompleted(USigilInputSystemComponent* IC, const FInputActionInstance& ActionData, FGameplayTag InputTag) const;

	/**
	 * Gets a friendly name for the editor.
	 * 获取编辑器友好的名称。
	 * @return The editor-friendly name. 编辑器友好的名称。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="InputProcessor")
	FString GetEditorFriendlyName() const;

#if WITH_EDITORONLY_DATA
	/**
	 * Friendly name for displaying in the editor.
	 * 在编辑器中显示的友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif

#if WITH_EDITOR
	/**
	 * Native implementation to get editor-friendly name.
	 * 获取编辑器友好名称的原生实现。
	 * @return The editor-friendly name. 编辑器友好的名称。
	 */
	FString NativeGetEditorFriendlyName() const;

	/**
	 * Called before saving the object.
	 * 在保存对象之前调用。
	 * @param SaveContext The save context. 保存上下文。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};