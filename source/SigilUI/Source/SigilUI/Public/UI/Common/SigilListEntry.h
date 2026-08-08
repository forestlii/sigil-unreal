// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/Foundation/SigilButtonBase.h"
#include "SigilListEntry.generated.h"

class USigilUIAction;

/**
 * List entry widget representing an item in a ListView or TileView.
 * 表示ListView或TileView中项的列表入口小部件。
 */
UCLASS(Abstract, meta = (Category = "Generic UI", DisableNativeTick))
class SIGILUI_API USigilListEntry : public USigilButtonBase, public IUserObjectListEntry
{
	GENERATED_BODY()
};