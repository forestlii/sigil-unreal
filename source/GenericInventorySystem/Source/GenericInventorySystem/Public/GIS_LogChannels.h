// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

/**
 * Gets the context string for logging purposes.
 * 获取用于日志记录的上下文字符串。
 * @param ContextObject The object providing the context (optional). 提供上下文的对象（可选）。
 * @return The context string. 上下文字符串。
 */
FString GetGISLogContextString(const UObject* ContextObject = nullptr);

/**
 * Log category for general inventory system messages.
 * 通用库存系统消息的日志类别。
 */
GENERICINVENTORYSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogGIS, Log, All);

/**
 * Macro for logging inventory system messages.
 * 用于记录库存系统消息的宏。
 * @details Logs messages with function name and formatted message. 记录包含函数名和格式化消息的日志。
 */
#define GIS_LOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogGIS, Verbosity, TEXT("%S: %s"),__FUNCTION__, *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}

/**
 * Macro for context-based logging for inventory system.
 * 用于库存系统的基于上下文的日志记录宏。
 * @details Logs messages with function name, context, and formatted message. 记录包含函数名、上下文和格式化消息的日志。
 */
#define GIS_CLOG(Verbosity, Format, ...) \
{ \
UE_LOG(LogGIS, Verbosity, TEXT("%S: ctx(%s) %s"),__FUNCTION__, *GetGISLogContextString(this), *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}

/**
 * Macro for context-based logging with an explicit owner.
 * 使用显式拥有者进行基于上下文的日志记录宏。
 * @details Logs messages with function name, owner context, and formatted message. 记录包含函数名、拥有者上下文和格式化消息的日志。
 */
#define GIS_OWNED_CLOG(LogOwner, Verbosity, Format, ...) \
{ \
UE_LOG(LogGIS, Verbosity, TEXT("%S: ctx(%s) %s"),__FUNCTION__, *GetGISLogContextString(LogOwner), *FString::Printf(TEXT(Format), ##__VA_ARGS__)) \
}