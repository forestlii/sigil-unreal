// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilButtonBase.h"
#include "SigilTabListWidgetBase.h"
#include "SigilTabButtonBase.generated.h"

class UCommonLazyImage;

/**
 * Button used for switching between tabs.
 * 用于切换选项卡的按钮。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Generic UI", DisableNativeTick))
class SIGILUI_API USigilTabButtonBase : public USigilButtonBase, public ISigilTabButtonInterface
{
	GENERATED_BODY()
};