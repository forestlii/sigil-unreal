// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputTriggers.h"
#include "Templates/TypeHash.h"
#include "UObject/Object.h"
#include "GIPS_InputTypes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGIPS_InputComponentSignature, UEnhancedInputComponent*, InputComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGIPS_InputBufferWindowStateChangedSignature, FGameplayTag, BufferWindowName, bool, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGIPS_ReceivedInputSignature, const FInputActionInstance&, ActionData, const FGameplayTag&, InputTag, ETriggerEvent, TriggerEvent);

enum class ETriggerEvent : uint8;
class UGIPS_InputProcessor;

/**
 * Defines the type of input buffering behavior.
 * 定义输入缓冲行为的类型。
 */
UENUM()
enum class EGIPS_InputBufferType : uint8
{
	/**
	 * Only the last input is executed when the buffer window closes.
	 * 仅在缓冲窗口关闭时执行最后按下的输入。
	 */
	LastInput,

	/**
	 * Inputs are executed immediately during the buffer window.
	 * 在缓冲窗口期间立即执行输入。
	 */
	Instant,

	/**
	 * Inputs with higher priority (earlier in the list) are executed first.
	 * 优先级较高的输入（列表中较早的）优先执行。
	 */
	HighestPriority
};

/**
 * Defines how input processors are executed for a single input event.
 * 定义单个输入事件的处理器执行方式。
 */
UENUM()
enum class EGIPS_InputProcessorExecutionType : uint8
{
	/**
	 * All valid processors are executed sequentially in top-to-bottom order.
	 * 所有有效处理器按从上到下的顺序依次执行。
	 */
	MatchAll,

	/**
	 * Only the first valid input processor is executed.
	 * 仅执行第一个有效的输入处理器。
	 */
	FirstOnly
};

/**
 * Struct for defining allowed inputs and their trigger events.
 * 定义允许的输入及其触发事件的结构体。
 */
USTRUCT()
struct GENERICINPUTSYSTEM_API FGIPS_AllowedInput
{
	GENERATED_BODY()

	/**
	 * The allowed input tag.
	 * 允许的输入标签。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input", meta=(Categories="InputTag,GIPS.InputTag"))
	FGameplayTag InputTag;

	/**
	 * Allowed trigger event types (all allowed if empty).
	 * 允许的触发事件类型（如果为空则允许所有）。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	TArray<ETriggerEvent> TriggerEvents{ETriggerEvent::Started};

	/**
	 * Equality operator for allowed inputs.
	 * 允许输入的相等比较运算符。
	 */
	friend bool operator==(const FGIPS_AllowedInput& Lhs, const FGIPS_AllowedInput& RHS)
	{
		return Lhs.InputTag == RHS.InputTag;
	}

	/**
	 * Inequality operator for allowed inputs.
	 * 允许输入的不相等比较运算符。
	 */
	friend bool operator!=(const FGIPS_AllowedInput& Lhs, const FGIPS_AllowedInput& RHS)
	{
		return !(Lhs == RHS);
	}
};

/**
 * Struct for defining an input buffer window.
 * 定义输入缓冲窗口的结构体。
 */
USTRUCT()
struct GENERICINPUTSYSTEM_API FGIPS_InputBufferWindow
{
	GENERATED_BODY()

public:
	/**
	 * The tag for the buffer window.
	 * 缓冲窗口的标签。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	FGameplayTag Tag = FGameplayTag::EmptyTag;

	/**
	 * The type of input buffering behavior.
	 * 输入缓冲行为的类型。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	EGIPS_InputBufferType BufferType = EGIPS_InputBufferType::LastInput;

	/**
	 * List of allowed inputs for the buffer window.
	 * 缓冲窗口允许的输入列表。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input", meta=(TitleProperty="InputTag"))
	TArray<FGIPS_AllowedInput> AllowedInputs;

	/**
	 * Finds the index of an allowed input.
	 * 查找允许输入的索引。
	 * @param InputTag The input tag to find. 要查找的输入标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return The index of the allowed input, or INDEX_NONE if not found. 允许输入的索引，如果未找到则返回INDEX_NONE。
	 */
	int32 IndexOfAllowedInput(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const
	{
		for (int32 i = 0; i < AllowedInputs.Num(); i++)
		{
			if (AllowedInputs[i].InputTag == InputTag && (AllowedInputs[i].TriggerEvents.IsEmpty() || AllowedInputs[i].TriggerEvents.Contains(TriggerEvent)))
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	/**
	 * Equality operator for buffer window tags.
	 * 缓冲窗口标签的相等比较运算符。
	 */
	bool operator==(const FGameplayTag& OtherTag) const;

	/**
	 * Inequality operator for buffer window tags.
	 * 缓冲窗口标签的不相等比较运算符。
	 */
	bool operator!=(const FGameplayTag& OtherTag) const;
};

/**
 * Struct for storing a single input entry.
 * 存储单个输入记录的结构体。
 */
USTRUCT(BlueprintType, meta=(DisplayName="GIPS Input Entry"))
struct GENERICINPUTSYSTEM_API FGIPS_BufferedInput
{
	GENERATED_BODY()

	/**
	 * The input tag for the entry.
	 * 输入记录的输入标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIPS|Input")
	FGameplayTag InputTag{FGameplayTag::EmptyTag};

	/**
	 * The input action data.
	 * 输入动作数据。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIPS|Input")
	FInputActionInstance ActionData;

	/**
	 * The trigger event type.
	 * 触发事件类型。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIPS|Input")
	ETriggerEvent TriggerEvent{ETriggerEvent::None};

	/**
	 * Equality operator for buffered inputs.
	 * 缓冲输入的相等比较运算符。
	 */
	friend bool operator==(const FGIPS_BufferedInput& Lhs, const FGIPS_BufferedInput& RHS)
	{
		return Lhs.InputTag == RHS.InputTag
			&& Lhs.TriggerEvent == RHS.TriggerEvent;
	}

	/**
	 * Inequality operator for buffered inputs.
	 * 缓冲输入的不相等比较运算符。
	 */
	friend bool operator!=(const FGIPS_BufferedInput& Lhs, const FGIPS_BufferedInput& RHS)
	{
		return !(Lhs == RHS);
	}

	FGIPS_BufferedInput() = default;

	/**
	 * Constructor for buffered input.
	 * 缓冲输入的构造函数。
	 */
	FGIPS_BufferedInput(const FGameplayTag& InputTag, const FInputActionInstance& ActionData, const ETriggerEvent TriggerEvent)
		: InputTag(InputTag),
		  ActionData(ActionData),
		  TriggerEvent(TriggerEvent)
	{
	}

	/**
	 * Copy constructor for buffered input.
	 * 缓冲输入的复制构造函数。
	 */
	FGIPS_BufferedInput(const FGIPS_BufferedInput& Other)
		: InputTag(Other.InputTag),
		  ActionData(Other.ActionData),
		  TriggerEvent(Other.TriggerEvent)
	{
	}

	/**
	 * Assignment operator for buffered input.
	 * 缓冲输入的赋值运算符。
	 */
	FGIPS_BufferedInput& operator=(const FGIPS_BufferedInput& Other)
	{
		if (this == &Other)
			return *this;
		InputTag = Other.InputTag;
		ActionData = Other.ActionData;
		TriggerEvent = Other.TriggerEvent;
		return *this;
	}

	/**
	 * Converts the buffered input to a string.
	 * 将缓冲输入转换为字符串。
	 * @return The string representation of the buffered input. 缓冲输入的字符串表示。
	 */
	FString ToString() const;
};

/**
 * Struct for defining input action settings.
 * 定义输入动作设置的结构体。
 */
USTRUCT()
struct GENERICINPUTSYSTEM_API FGIPS_InputActionSetting
{
	GENERATED_BODY()

	/**
	 * The input action object.
	 * 输入动作对象。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	TObjectPtr<UInputAction> InputAction;

	/**
	 * Enables value binding for the input action.
	 * 为输入动作启用值绑定。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	bool bValueBinding{true};
};

/**
 * Struct for defining relationships between actor states and input permissions.
 * 定义演员状态与输入权限关系的结构体。
 */
USTRUCT()
struct GENERICINPUTSYSTEM_API FGIPS_InputTagRelationship
{
	GENERATED_BODY()

	/**
	 * Query to check if the actor's tags allow the input.
	 * 检查演员标签是否允许输入的查询。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input")
	FGameplayTagQuery ActorTagQuery;

	/**
	 * List of allowed inputs if the tag query is satisfied.
	 * 如果标签查询满足，则允许的输入列表。
	 */
	UPROPERTY(EditAnywhere, Category="GIPS|Input", meta=(TitleProperty="InputTag"))
	TArray<FGIPS_AllowedInput> AllowedInputs;

	/**
	 * Finds the index of an allowed input.
	 * 查找允许输入的索引。
	 * @param InputTag The input tag to find. 要查找的输入标签。
	 * @param TriggerEvent The trigger event type. 触发事件类型。
	 * @return The index of the allowed input, or INDEX_NONE if not found. 允许输入的索引，如果未找到则返回INDEX_NONE。
	 */
	int32 IndexOfAllowedInput(const FGameplayTag& InputTag, const ETriggerEvent& TriggerEvent) const
	{
		for (int32 i = 0; i < AllowedInputs.Num(); i++)
		{
			if (AllowedInputs[i].InputTag == InputTag && (AllowedInputs[i].TriggerEvents.IsEmpty() || AllowedInputs[i].TriggerEvents.Contains(TriggerEvent)))
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

#if WITH_EDITORONLY_DATA
	/**
	 * Friendly name for displaying in the editor.
	 * 在编辑器中显示的友好名称。
	 */
	UPROPERTY(VisibleAnywhere, Category=AlwaysHidden, Meta=(EditCondition=False, EditConditionHides))
	FString EditorFriendlyName;
#endif
};