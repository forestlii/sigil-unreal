// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

/**
 * Gets the context string for logging purposes.
 * 获取用于日志记录的上下文字符串。
 * @param ContextObject The object providing the context (optional). 提供上下文的对象（可选）。
 * @return The context string. 上下文字符串。
 */
FString GetGGSLogContextString(const UObject* ContextObject = nullptr);

/**
 * Log category for generic game system messages.
 * 通用游戏系统消息的日志类别。
 */
SIGILINTERACTION_API DECLARE_LOG_CATEGORY_EXTERN(LogSigilInteraction, Log, All);

/**
 * Macro for logging generic game system messages.
 * 用于记录通用游戏系统消息的宏。
 * @details Logs messages with function name and formatted message. 记录包含函数名和格式化消息的日志。
 */
#define SIGIL_INTERACTION_LOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogSigilInteraction, Verbosity, TEXT("%S: %s"),__FUNCTION__, *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}

/**
 * Macro for context-based logging for generic game system.
 * 用于通用游戏系统的基于上下文的日志记录宏。
 * @details Logs messages with function name, context, and formatted message. 记录包含函数名、上下文和格式化消息的日志。
 */
#define SIGIL_INTERACTION_CLOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogSigilInteraction, Verbosity, TEXT("%S: ctx(%s) %s"),__FUNCTION__, *GetGGSLogContextString(this), *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}

/**
 * Macro for context-based logging with an explicit owner.
 * 使用显式拥有者进行基于上下文的日志记录宏。
 * @details Logs messages with function name, owner context, and formatted message. 记录包含函数名、拥有者上下文和格式化消息的日志。
 */
#define SIGIL_INTERACTION_OWNED_CLOG(LogOwner, Verbosity, Format, ...) \
{ \
UE_LOG(LogSigilInteraction, Verbosity, TEXT("%S: ctx(%s) %s"),__FUNCTION__, *GetGGSLogContextString(LogOwner), *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}
