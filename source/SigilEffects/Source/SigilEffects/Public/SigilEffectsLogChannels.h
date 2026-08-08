// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"

/**
 * Gets the context string for logging purposes.
 * 获取用于日志记录的上下文字符串。
 * @param ContextObject The object providing the context (optional). 提供上下文的对象（可选）。
 * @return The context string. 上下文字符串。
 */
FString GetGESLogContextString(const UObject* ContextObject = nullptr);

/**
 * Log category for generic effects system messages.
 * 通用效果系统消息的日志类别。
 */
SIGILEFFECTS_API DECLARE_LOG_CATEGORY_EXTERN(LogSigilEffects, Log, All);

/**
 * Macro for logging effects system messages.
 * 用于记录效果系统消息的宏。
 * @details Logs messages with function name and formatted message. 记录包含函数名和格式化消息的日志。
 */
#define SIGIL_EFFECTS_LOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogSigilEffects, Verbosity, TEXT("%S: %s"),__FUNCTION__, *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}

/**
 * Macro for context-based logging for effects system.
 * 用于效果系统的基于上下文的日志记录宏。
 * @details Logs messages with function name, context, and formatted message. 记录包含函数名、上下文和格式化消息的日志。
 */
#define SIGIL_EFFECTS_CLOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogSigilEffects, Verbosity, TEXT("%S: ctx(%s) %s"),__FUNCTION__, *GetGESLogContextString(this), *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}

/**
 * Macro for context-based logging with an explicit owner.
 * 使用显式拥有者进行基于上下文的日志记录宏。
 * @details Logs messages with function name, owner context, and formatted message. 记录包含函数名、拥有者上下文和格式化消息的日志。
 */
#define SIGIL_EFFECTS_OWNED_CLOG(LogOwner, Verbosity, Format, ...) \
{ \
UE_LOG(LogSigilEffects, Verbosity, TEXT("%S: ctx(%s) %s"),__FUNCTION__, *GetGESLogContextString(LogOwner), *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}
