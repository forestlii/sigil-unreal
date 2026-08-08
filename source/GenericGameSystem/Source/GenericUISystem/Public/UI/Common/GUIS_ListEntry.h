// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/Foundation/GUIS_ButtonBase.h"
#include "GUIS_ListEntry.generated.h"

class UGUIS_UIAction;

/**
 * List entry widget representing an item in a ListView or TileView.
 * 表示ListView或TileView中项的列表入口小部件。
 */
UCLASS(Abstract, meta = (Category = "Generic UI", DisableNativeTick))
class GENERICUISYSTEM_API UGUIS_ListEntry : public UGUIS_ButtonBase, public IUserObjectListEntry
{
	GENERATED_BODY()
};