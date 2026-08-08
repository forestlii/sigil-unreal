// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "Engine/CancellableAsyncAction.h"
#include "SigilInputTypes.h"
#include "SigilAsyncAction_ListenInputEvent.generated.h"

class USigilInputSystemComponent;

/**
 * Async action for listening to input events.
 * 用于监听输入事件的异步动作。
 */
UCLASS()
class SIGILINPUT_API USigilAsyncAction_ListenInputEvent : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Creates an async action to listen for input events.
	 * 创建一个异步动作以监听输入事件。
	 * @param WorldContextObject The world context object. 世界上下文对象。
	 * @param InputSystemComponent The input system component. 输入系统组件。
	 * @param InputTagsToListen Tags to listen for. 要监听的标签。
	 * @param EventsToListen Trigger events to listen for. 要监听的触发事件。
	 * @param bListenForBufferedInput Whether to listen for buffered inputs. 是否监听缓冲输入。
	 * @param bExactMatch Whether to require exact tag matching. 是否要求精确标签匹配。
	 * @return The async action instance. 异步动作实例。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input", meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static USigilAsyncAction_ListenInputEvent* ListenInputEvent(UObject* WorldContextObject, USigilInputSystemComponent* InputSystemComponent, FGameplayTagContainer InputTagsToListen,
	                                                           TArray<ETriggerEvent> EventsToListen, bool bListenForBufferedInput = false, bool bExactMatch = true);

	/**
	 * Activates the async action.
	 * 激活异步动作。
	 */
	virtual void Activate() override;

	/**
	 * Event triggered when an input is received.
	 * 接收到输入时触发的事件。
	 */
	UPROPERTY(BlueprintAssignable)
	FSigilReceivedInputSignature OnReceivedInput;

	/**
	 * Handles the input event.
	 * 处理输入事件。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 */
	UFUNCTION()
	void HandleInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent);

private:
	/**
	 * Cancels the async action.
	 * 取消异步动作。
	 */
	virtual void Cancel() override;

	/**
	 * Tags to listen for.
	 * 要监听的标签。
	 */
	FGameplayTagContainer InputTags;

	/**
	 * Trigger events to listen for.
	 * 要监听的触发事件。
	 */
	TArray<ETriggerEvent> TriggerEvents;

	/**
	 * Whether to listen for buffered inputs.
	 * 是否监听缓冲输入。
	 */
	bool bForBufferedInput{false};

	/**
	 * Whether to require exact tag matching.
	 * 是否要求精确标签匹配。
	 */
	bool bExact{true};

	/**
	 * Weak reference to the input system component.
	 * 输入系统组件的弱引用。
	 */
	TWeakObjectPtr<USigilInputSystemComponent> Input;
};