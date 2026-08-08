// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIPS_InputTypes.h"
#include "GIPS_InputControlSetup.generated.h"

struct FInputActionInstance;
class UGIPS_InputChecker;
class UGIPS_InputProcessor;
class UGIPS_InputSystemComponent;

/**
 * Data asset for defining input checkers and processors.
 * 定义输入检查器和处理器的数据资产。
 */
UCLASS(BlueprintType, Const)
class GENERICINPUTSYSTEM_API UGIPS_InputControlSetup : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Handles input actions for the input system.
	 * 为输入系统处理输入动作。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 */
	void HandleInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent) const;

	/**
	 * Checks if an input is allowed.
	 * 检查输入是否被允许。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input is allowed, false otherwise. 如果输入被允许则返回true，否则返回false。
	 */
	bool CheckInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent);

protected:
	/**
	 * Determines if debugging is enabled for the input.
	 * 确定是否为输入启用调试。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if debugging is enabled, false otherwise. 如果启用调试则返回true，否则返回false。
	 */
	bool ShouldDebug(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const;

	/**
	 * Internal logic for checking input validity.
	 * 检查输入有效性的内部逻辑。
	 * @param IC The input system component. 输入系统组件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input is valid, false otherwise. 如果输入有效则返回true，否则返回false。
	 */
	virtual bool InternalCheckInput(UGIPS_InputSystemComponent* IC, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent);

	/**
	 * Filters input processors based on the input tag and trigger event.
	 * 根据输入标签和触发事件过滤输入处理器。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return Array of filtered input processors. 过滤后的输入处理器数组。
	 */
	TArray<TObjectPtr<UGIPS_InputProcessor>> FilterInputProcessors(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const;

	/**
	 * List of input checkers to validate input events.
	 * 验证输入事件的一组输入检查器。
	 */
	UPROPERTY(EditAnywhere, Instanced, Category="GIPS|Input")
	TArray<TObjectPtr<UGIPS_InputChecker>> InputCheckers;

	/**
	 * If enabled, disallowed inputs are attempted to be stored in the input buffer.
	 * 如果启用，不允许的输入将尝试存储在输入缓冲区中。
	 * @attention Input buffering may not be needed for some setups, like UI. 对于某些设置（如UI），可能不需要输入缓冲。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	bool bEnableInputBuffer{false};

	/**
	 * Controls the execution order of input processors for a single input event.
	 * 控制单个输入事件的处理器执行顺序。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	EGIPS_InputProcessorExecutionType InputProcessorExecutionType{EGIPS_InputProcessorExecutionType::MatchAll};

	/**
	 * List of input processors to handle allowed input events sequentially.
	 * 处理允许的输入事件的一组输入处理器，按顺序执行。
	 */
	UPROPERTY(EditAnywhere, Instanced, Category="GIPS|Input", meta=(TitleProperty="EditorFriendlyName", ShowOnlyInnerProperties))
	TArray<TObjectPtr<UGIPS_InputProcessor>> InputProcessors;

	/**
	 * Enables debug logging for input events.
	 * 为输入事件启用调试日志。
	 * @attention Requires LogGIPS to be set to VeryVerbose to take effect. 需要将LogGIPS设置为VeryVerbose才能生效。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Debug")
	bool bEnableInputDebug{false};

	/**
	 * Input tags to debug (logs all tags if empty).
	 * 要调试的输入标签（如果为空则记录所有标签）。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Debug", meta=(EditCondition="bEnableInputDebug", Categories="InputTag,GIPS.InputTag"))
	FGameplayTagContainer DebugInputTags{};

	/**
	 * Trigger events to debug (logs all events if empty).
	 * 要调试的触发事件（如果为空则记录所有事件）。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Debug", meta=(EditCondition="bEnableInputDebug"))
	TArray<ETriggerEvent> DebugTriggerEvents{ETriggerEvent::Started, ETriggerEvent::Completed};

public:
#if WITH_EDITOR
	/**
	 * Validates the data asset in the editor.
	 * 在编辑器中验证数据资产。
	 * @param Context The data validation context. 数据验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};