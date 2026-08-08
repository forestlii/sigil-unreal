// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "GIPS_InputProcessor.h"
#include "GIPS_InputTypes.h"
#include "GIPS_InputSystemComponent.generated.h"

class UGIPS_InputControlSetup;
class UInputMappingContext;
class UGIPS_InputConfig;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * Core component for handling input in the Generic Input System.
 * 通用输入系统中处理输入的核心组件。
 * @attention Must be attached to a Pawn or PlayerController. 必须挂载到Pawn或PlayerController上。
 */
UCLASS(ClassGroup=GIPS, Blueprintable, meta=(BlueprintSpawnableComponent), AutoExpandCategories=("GIPS"))
class GENERICINPUTSYSTEM_API UGIPS_InputSystemComponent : public UActorComponent
{
	GENERATED_BODY()

	friend UGIPS_InputControlSetup;

public:
	enum class EGIPS_OwnerType:uint8
	{
		Pawn,
		PC,
	};

public:
	UGIPS_InputSystemComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorComponent interface
	/**
	 * Called when the component is registered.
	 * 组件注册时调用。
	 */
	virtual void OnRegister() override;

	/**
	 * Called when the component is unregistered.
	 * 组件取消注册时调用。
	 */
	virtual void OnUnregister() override;
	//~ End UActorComponent interface

	/**
	 * Gets the Pawn associated with this component.
	 * 获取与此组件关联的Pawn。
	 * @return The associated Pawn, or the Pawn controlled by the PlayerController if the owner is a PlayerController. 如果拥有者是Pawn则返回Pawn，如果是PlayerController则返回其控制的Pawn。
	 */
	UFUNCTION(BlueprintPure, Category="GIPS|Input")
	APawn* GetControlledPawn() const;

	/**
	 * Gets the input system component from an actor.
	 * 从演员获取输入系统组件。
	 * @param Actor The actor to query. 要查询的演员。
	 * @return The input system component, or nullptr if not found. 输入系统组件，如果未找到则返回nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GIPS|Input", Meta = (DefaultToSelf="Actor"))
	static UGIPS_InputSystemComponent* GetInputSystemComponent(const AActor* Actor);

	/**
	 * Finds the input system component on an actor.
	 * 在演员上查找输入系统组件。
	 * @param Actor The actor to query. 要查询的演员。
	 * @param Component The found component (output). 找到的组件（输出）。
	 * @return True if the component was found, false otherwise. 如果找到组件则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GIPS|Input", Meta = (DefaultToSelf="Actor", ExpandBoolAsExecs="ReturnValue"))
	static bool FindInputSystemComponent(const AActor* Actor, UGIPS_InputSystemComponent*& Component);

	/**
	 * Event triggered when setting up the input component.
	 * 设置输入组件时触发的事件。
	 */
	UPROPERTY(BlueprintAssignable, Category="GIPS|Input")
	FGIPS_InputComponentSignature SetupInputComponentEvent;

	/**
	 * Event triggered when cleaning up the input component.
	 * 清理输入组件时触发的事件。
	 */
	UPROPERTY(BlueprintAssignable, Category="GIPS|Input")
	FGIPS_InputComponentSignature CleanupInputComponentEvent;

	/**
	 * Event triggered when the input buffer window state changes.
	 * 输入缓冲窗口状态改变时触发的事件。
	 */
	UPROPERTY(BlueprintAssignable, Category="GIPS|Input")
	FGIPS_InputBufferWindowStateChangedSignature InputBufferWindowStateChangedEvent;

protected:
	/**
	 * Sets up the player input component (Blueprint-implementable).
	 * 设置玩家输入组件（可通过蓝图实现）。
	 * @param NewInputComponent The new input component. 新输入组件。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIPS|Input")
	void OnSetupPlayerInputComponent(UEnhancedInputComponent* NewInputComponent);
	virtual void OnSetupPlayerInputComponent_Implementation(UEnhancedInputComponent* NewInputComponent);

	/**
	 * Cleans up the player input component (Blueprint-implementable).
	 * 清理玩家输入组件（可通过蓝图实现）。
	 * @param PrevInputComponent The previous input component. 前一个输入组件。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIPS|Input")
	void OnCleanupPlayerInputComponent(UEnhancedInputComponent* PrevInputComponent);
	virtual void OnCleanupPlayerInputComponent_Implementation(UEnhancedInputComponent* PrevInputComponent);

	/**
	 * Called when the pawn restarts.
	 * 当Pawn重启时调用。
	 * @param Pawn The pawn that restarted. 重启的Pawn。
	 */
	UFUNCTION()
	virtual void OnPawnRestarted(APawn* Pawn);

	/**
	 * Called when the controller changes.
	 * 当控制器更改时调用。
	 * @param Pawn The associated pawn. 关联的Pawn。
	 * @param OldController The previous controller. 前一个控制器。
	 * @param NewController The new controller. 新控制器。
	 */
	UFUNCTION()
	virtual void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	/**
	 * Cleans up input action value bindings.
	 * 清理输入动作值绑定。
	 */
	void CleanInputActionValueBindings();

	/**
	 * Sets up input action value bindings.
	 * 设置输入动作值绑定。
	 */
	void SetupInputActionValueBindings();

	/**
	 * Sets up the input component.
	 * 设置输入组件。
	 * @param NewInputComponent The new input component. 新输入组件。
	 */
	virtual void SetupInputComponent(UInputComponent* NewInputComponent);

	/**
	 * Cleans up the input component.
	 * 清理输入组件。
	 * @param OldController The previous controller (optional). 前一个控制器（可选）。
	 */
	virtual void CleanupInputComponent(AController* OldController = nullptr);

	/**
	 * Gets the enhanced input subsystem.
	 * 获取增强输入子系统。
	 * @param OldController The previous controller (optional). 前一个控制器（可选）。
	 * @return The enhanced input subsystem. 增强输入子系统。
	 */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(AController* OldController = nullptr) const;

	/**
	 * The bound input component.
	 * 绑定的输入组件。
	 */
	UPROPERTY(transient)
	TObjectPtr<UEnhancedInputComponent> InputComponent;

	/**
	 * The type of owner (Pawn or PlayerController).
	 * 拥有者类型（Pawn或PlayerController）。
	 */
	EGIPS_OwnerType OwnerType = EGIPS_OwnerType::PC;

protected:
	/**
	 * Binds input actions to the component.
	 * 将输入动作绑定到组件。
	 */
	void BindInputActions();

	/**
	 * Input mapping context to be added to the enhanced input subsystem.
	 * 将添加到增强输入子系统的输入映射上下文。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIPS|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	/**
	 * Priority for binding the input mapping context.
	 * 绑定输入映射上下文的优先级。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIPS|Input")
	int32 InputPriority = 0;

	/**
	 * Main configuration for the input system.
	 * 输入系统的主配置。
	 */
	UPROPERTY(EditDefaultsOnly, Category="GIPS|Input")
	TObjectPtr<UGIPS_InputConfig> InputConfig;

	/**
	 * Maximum number of input entries to keep.
	 * 保留的最大输入记录数。
	 */
	UPROPERTY(EditDefaultsOnly, Category="GIPS|Input", meta=(ClampMin=0, ClampMax=10))
	int32 MaxInputEntriesNum{5};

	/**
	 * If true, inputs are processed externally via OnReceivedInput event.
	 * 如果为true，输入通过OnReceivedInput事件在外部处理。
	 */
	UPROPERTY(EditDefaultsOnly, Category="GIPS|Input")
	bool bProcessingInputExternally{false};

	/**
	 * List of input control setups, with the last one being the current setup.
	 * 输入控制设置列表，最后一个为当前设置。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	TArray<TObjectPtr<UGIPS_InputControlSetup>> InputControlSetups;

public:
#pragma region InputControl
	/**
	 * Gets the current input control setup.
	 * 获取当前输入控制设置。
	 * @return The last input control setup in the list. 列表中的最后一个输入控制设置。
	 */
	UFUNCTION(BlueprintPure, Category="GIPS|Input")
	UGIPS_InputControlSetup* GetCurrentInputSetup() const;

	/**
	 * Gets the input configuration.
	 * 获取输入配置。
	 * @return The input configuration. 输入配置。
	 */
	UFUNCTION(BlueprintPure, Category="GIPS|Input")
	UGIPS_InputConfig* GetInputConfig() const;

	/**
	 * Sets a new input control setup as the current one.
	 * 将新的输入控制设置设为当前设置。
	 * @param NewSetup The new input control setup. 新输入控制设置。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	virtual void PushInputSetup(UGIPS_InputControlSetup* NewSetup);

	/**
	 * Removes the current input setup, making the last one in the list the new current setup.
	 * 删除当前输入设置，使列表中的最后一个成为新的当前设置。
	 * @attention Does nothing if only one setup exists. 如果只有一个设置，则无操作。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	virtual void PopInputSetup();

#pragma endregion

#pragma region InputTag
	/**
	 * Checks if an input is allowed.
	 * 检查输入是否被允许。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input is allowed, false otherwise. 如果输入被允许则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input", meta=(ExpandBoolAsExecs="ReturnValue"))
	virtual bool CheckInputAllowed(UPARAM(meta = (Categories="InputTag,GIPS.InputTag")) FGameplayTag InputTag, ETriggerEvent TriggerEvent);

	/**
	 * Checks if an input is allowed with action data.
	 * 使用动作数据检查输入是否被允许。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input is allowed, false otherwise. 如果输入被允许则返回true，否则返回false。
	 */
	// UFUNCTION(BlueprintCallable, Category="GIPS|Input", meta=(ExpandBoolAsExecs="ReturnValue", AutoCreateRefTerm="ActionData"))
	virtual bool CheckInputAllowed(const FInputActionInstance& ActionData, UPARAM(meta = (Categories="InputTag,GIPS.InputTag")) FGameplayTag InputTag, ETriggerEvent TriggerEvent);

	/**
	 * Processes an input action.
	 * 处理输入动作。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 */
	// UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	virtual void ProcessInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent);

	/**
	 * Gets the input action associated with a tag.
	 * 获取与标签关联的输入动作。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @return The associated input action. 关联的输入动作。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	UInputAction* GetInputActionOfInputTag(UPARAM(meta = (Categories="InputTag,GIPS.InputTag")) FGameplayTag InputTag) const;

	/**
	 * Gets the current input action value for a tag.
	 * 获取标签的当前输入动作值。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @return The current input action value. 当前输入动作值。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	FInputActionValue GetInputActionValueOfInputTag(UPARAM(meta = (Categories="InputTag,GIPS.InputTag")) FGameplayTag InputTag) const;

	/**
	 * Gets the last input action value for a tag.
	 * 获取标签的最后输入动作值。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @return The last input action value. 最后输入动作值。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	FInputActionValue GetLastInputActionValueOfInputTag(UPARAM(meta = (Categories="InputTag,GIPS.InputTag")) FGameplayTag InputTag) const;

	/**
	 * Gets the list of passed input entries.
	 * 获取通过的输入记录列表。
	 * @return Array of passed input entries. 通过的输入记录数组。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	TArray<FGIPS_BufferedInput> GetPassedInputEntries() const { return PassedInputEntries; };

	/**
	 * Registers a passed input entry.
	 * 注册通过的输入记录。
	 * @param InputEntry The input entry to register. 要注册的输入记录。
	 */
	virtual void RegisterPassedInputEntry(const FGIPS_BufferedInput& InputEntry);

	/**
	 * Gets the list of blocked input entries.
	 * 获取阻止的输入记录列表。
	 * @return Array of blocked input entries. 阻止的输入记录数组。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	TArray<FGIPS_BufferedInput> GetBlockedInputEntries() const { return BlockedInputEntries; };

	/**
	 * Registers a blocked input entry.
	 * 注册阻止的输入记录。
	 * @param InputEntry The input entry to register. 要注册的输入记录。
	 */
	virtual void RegisterBlockedInputEntry(const FGIPS_BufferedInput& InputEntry);

	/**
	 * Gets the list of buffered input entries.
	 * 获取缓冲的输入记录列表。
	 * @return Array of buffered input entries. 缓冲的输入记录数组。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|Input")
	TArray<FGIPS_BufferedInput> GetBufferedInputEntries() const { return BufferedInputEntries; };

	/**
	 * Registers a buffered input entry.
	 * 注册缓冲的输入记录。
	 * @param InputEntry The input entry to register. 要注册的输入记录。
	 */
	virtual void RegisterBufferedInputEntry(const FGIPS_BufferedInput& InputEntry);

	/**
	 * Event triggered when an input action generates an event.
	 * 输入动作生成事件时触发的事件。
	 */
	UPROPERTY(BlueprintAssignable)
	FGIPS_ReceivedInputSignature OnReceivedInput;

protected:
	/**
	 * Callback for input action events.
	 * 输入动作事件的回调。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 */
	UFUNCTION()
	void InputActionCallback(const FInputActionInstance& ActionData, FGameplayTag InputTag, ETriggerEvent TriggerEvent);

	/**
	 * Mapping of input tags to action value bindings.
	 * 输入标签到动作值绑定的映射。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="GIPS|Input", meta=(ForceInlineRow))
	TMap<FGameplayTag, int32> InputActionValueBindings;

	/**
	 * Tracks the last action value for each input action.
	 * 跟踪每个输入动作的最后动作值。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="GIPS|Input", meta=(ForceInlineRow))
	TMap<FGameplayTag, FInputActionValue> LastInputActionValues;

	/**
	 * List of passed input entries.
	 * 通过的输入记录列表。
	 */
	UPROPERTY(VisibleAnywhere, Category="GIPS|Input")
	TArray<FGIPS_BufferedInput> PassedInputEntries;

	/**
	 * List of blocked input entries.
	 * 阻止的输入记录列表。
	 */
	UPROPERTY(VisibleAnywhere, Category="GIPS|Input")
	TArray<FGIPS_BufferedInput> BlockedInputEntries;

	/**
	 * List of buffered input entries.
	 * 缓冲的输入记录列表。
	 */
	UPROPERTY(VisibleAnywhere, Category="GIPS|Input")
	TArray<FGIPS_BufferedInput> BufferedInputEntries;

#pragma region InputBuffer
public:
	/**
	 * Attempts to save an input to the buffer.
	 * 尝试将输入保存到缓冲区。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input was saved, false otherwise. 如果输入被保存则返回true，否则返回false。
	 */
	bool TrySaveInput(const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent);

	/**
	 * Fires buffered input actions.
	 * 触发缓冲的输入动作。
	 */
	void FireBufferedInput();

	/**
	 * Event triggered when buffered input is fired.
	 * 缓冲输入触发时的事件。
	 */
	UPROPERTY(BlueprintAssignable)
	FGIPS_ReceivedInputSignature OnFireBufferedInput;

	/**
	 * Opens an input buffer window to allow input saving.
	 * 开启输入缓冲窗口以允许保存输入。
	 * @param BufferWindowName The name of the buffer window to open. 要开启的缓冲窗口名称。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|InputBuffer")
	virtual void OpenInputBufferWindow(FGameplayTag BufferWindowName);

	/**
	 * Closes an input buffer window.
	 * 关闭输入缓冲窗口。
	 * @param BufferWindowName The name of the buffer window to close. 要关闭的缓冲窗口名称。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|InputBuffer")
	virtual void CloseInputBufferWindow(FGameplayTag BufferWindowName);

	/**
	 * Closes all active input buffer windows.
	 * 关闭所有激活的输入缓冲窗口。
	 */
	UFUNCTION(BlueprintCallable, Category="GIPS|InputBuffer")
	virtual void CloseActiveInputBufferWindows();

	/**
	 * Gets the last buffered input.
	 * 获取最后缓冲的输入。
	 * @return The last buffered input. 最后缓冲的输入。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIPS|InputBuffer")
	FGIPS_BufferedInput GetLastBufferedInput() const;

	/**
	 * Gets all currently active buffer windows.
	 * 获取当前所有激活的缓冲窗口。
	 * @return Map of active buffer windows. 激活的缓冲窗口映射。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIPS|InputBuffer")
	TMap<FGameplayTag, FGIPS_BufferedInput> GetActiveBufferWindows() const;

protected:
	/**
	 * Resets the buffered input.
	 * 重置缓冲输入。
	 */
	void ResetBufferedInput();

	/**
	 * Attempts to save an input as a buffered input.
	 * 尝试将输入保存为缓冲输入。
	 * @param BufferWindowName The buffer window name. 缓冲窗口名称。
	 * @param ActionData The input action data. 输入动作数据。
	 * @param InputTag The gameplay tag for the input. 输入的游戏标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return True if the input was saved, false otherwise. 如果输入被保存则返回true，否则返回false。
	 */
	bool TrySaveAsBufferedInput(const FGameplayTag BufferWindowName, const FInputActionInstance& ActionData, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent);

	/**
	 * Map of currently active input buffer windows.
	 * 当前激活的输入缓冲窗口映射。
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="GIPS|InputBuffer", meta=(ForceInlineRow))
	TMap<FGameplayTag, FGIPS_BufferedInput> ActiveBufferWindows;

	/**
	 * The last fired buffered input.
	 * 最后触发的缓冲输入。
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="GIPS|InputBuffer")
	FGIPS_BufferedInput LastBufferedInput;

	/**
	 * The current buffered input.
	 * 当前缓冲输入。
	 */
	UPROPERTY()
	FGIPS_BufferedInput CurrentBufferedInput;

#pragma endregion

#pragma region DataValidation
#if WITH_EDITOR
	/**
	 * Validates the component data in the editor.
	 * 在编辑器中验证组件数据。
	 * @param Context The data validation context. 数据验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
#pragma endregion
};