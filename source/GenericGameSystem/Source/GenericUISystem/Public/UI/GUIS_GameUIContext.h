// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GUIS_GameUIContext.generated.h"

/**
 * Base class for UI context data shared across multiple UI elements.
 * 多个UI元素共享的UI上下文数据的基类。
 * @details Allows subclassing to add custom data for UI interactions.
 * @细节 允许子类化以添加用于UI交互的自定义数据。
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GENERICUISYSTEM_API UGUIS_GameUIContext : public UObject
{
	GENERATED_BODY()
};