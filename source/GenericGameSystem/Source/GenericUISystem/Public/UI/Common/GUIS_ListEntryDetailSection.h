// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GUIS_ListEntryDetailSection.generated.h"

/**
 * Detail section widget for composing a detail view.
 * 组成细节视图的细节部分小部件。
 */
UCLASS(Abstract, meta = (Category = "Generic UI"))
class GENERICUISYSTEM_API UGUIS_ListEntryDetailSection : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Sets the object represented by this detail section.
	 * 设置此细节部分表示的对象。
	 * @param ListItemObject The object to display. 要显示的对象。
	 */
	void SetListItemObject(UObject* ListItemObject);

protected:
	/**
	 * Called when the list item object is set.
	 * 列表项对象设置时调用。
	 * @param ListItemObject The object being set. 设置的对象。
	 */
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject);

	/**
	 * Blueprint event for when the list item object is set.
	 * 列表项对象设置时的蓝图事件。
	 * @param ListItemObject The object being set. 设置的对象。
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnListItemObjectSet(UObject* ListItemObject);
};