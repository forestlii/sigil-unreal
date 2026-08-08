// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SigilGameUIContext.generated.h"

/**
 * Base class for UI context data shared across multiple UI elements.
 * 多个UI元素共享的UI上下文数据的基类。
 * @details Allows subclassing to add custom data for UI interactions.
 * @细节 允许子类化以添加用于UI交互的自定义数据。
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SIGILUI_API USigilGameUIContext : public UObject
{
	GENERATED_BODY()
};