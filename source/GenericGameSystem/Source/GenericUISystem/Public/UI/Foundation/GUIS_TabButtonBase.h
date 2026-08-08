// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GUIS_ButtonBase.h"
#include "GUIS_TabListWidgetBase.h"
#include "GUIS_TabButtonBase.generated.h"

class UCommonLazyImage;

/**
 * Button used for switching between tabs.
 * 用于切换选项卡的按钮。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Generic UI", DisableNativeTick))
class GENERICUISYSTEM_API UGUIS_TabButtonBase : public UGUIS_ButtonBase, public IGUIS_TabButtonInterface
{
	GENERATED_BODY()
};