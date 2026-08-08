// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "GIS_CoreStructLibray.generated.h"

class UGIS_ItemInstance;
class UGIS_ItemDefinition;

/**
 * Structure representing an item definition with an associated amount.
 * 表示道具定义及其关联数量的结构体。
 */
USTRUCT(BlueprintType)
struct FGIS_ItemDefinitionAmount
{
	GENERATED_BODY()

	FGIS_ItemDefinitionAmount();


	FGIS_ItemDefinitionAmount(TSoftObjectPtr<UGIS_ItemDefinition> InDefinition, int32 InAmount);

	/**
	 * The item definition.
	 * 道具定义。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	TSoftObjectPtr<UGIS_ItemDefinition> Definition;

	/**
	 * The amount of the item.
	 * 道具数量。
	 * @attention Minimum value is 1.
	 * @注意 最小值为1。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS", meta=(ClampMin="1"))
	int32 Amount = 1;
};

/**
 * Structure representing a default loadout with a tag and associated items.
 * 表示默认装备配置的结构体，包含标签和关联的道具。
 */
USTRUCT(BlueprintType)
struct FGIS_DefaultLoadout
{
	GENERATED_BODY()

	/**
	 * The gameplay tag for the loadout.
	 * 装备配置的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	FGameplayTag Tag;

	/**
	 * List of default items for the loadout.
	 * 装备配置的默认道具列表。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GIS")
	TArray<FGIS_ItemDefinitionAmount> DefaultItems;
};

/**
 * Structure defining options for handling item overflow in an inventory.
 * 定义库存中道具溢出处理选项的结构体。
 */
USTRUCT()
struct GENERICINVENTORYSYSTEM_API FGIS_ItemOverflowOptions
{
	GENERATED_BODY()

	/**
	 * Whether to return overflow items to their source (if specified) when the inventory is full.
	 * 当库存满时，是否将溢出的道具返回其来源（如果指定）。
	 */
	UPROPERTY(EditAnywhere, Category=Overflow)
	bool bReturnOverflow = true;

	/**
	 * Whether to send a rejection message when items cannot be added due to overflow.
	 * 当道具因溢出无法添加时，是否发送拒绝消息。
	 */
	UPROPERTY(EditAnywhere, Category=Overflow)
	bool bSendRejectedMessage = true;
};

/**
 * Structure representing an item slot definition, primarily used in item slot collections.
 * 表示道具槽定义的结构体，主要用于道具槽集合。
 */
USTRUCT(BlueprintType)
struct GENERICINVENTORYSYSTEM_API FGIS_ItemSlotDefinition
{
	GENERATED_BODY()

	/**
	 * The gameplay tag for the item slot.
	 * 道具槽的游戏标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ItemSlot, meta=(Categories="Slots,GIS.Slots,GGF.Slots"))
	FGameplayTag Tag;

	/**
	 * The icon for the item slot.
	 * 道具槽的图标。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ItemSlot)
	TSoftObjectPtr<UTexture2D> Icon;

	/**
	 * The Slate brush for the item slot icon.
	 * 道具槽图标的Slate画刷。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ItemSlot)
	FSlateBrush IconBrush;

	/**
	 * The display name of the item slot.
	 * 道具槽的显示名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ItemSlot)
	FText Name;

	/**
	 * The description of the item slot.
	 * 道具槽的描述。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ItemSlot)
	FText Desc;

	/**
	 * Gameplay tag query that item tags must match to be equipped in this slot.
	 * 道具标签必须匹配的游戏标签查询，才能装备到此槽。
	 * @attention If empty, no items can be equipped in this slot.
	 * @注意 如果为空，则无法将任何道具装备到此槽。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ItemSlot)
	FGameplayTagQuery TagQuery;

	/**
	 * Checks if an item can be equipped in this slot based on its tags.
	 * 检查道具是否可以根据其标签装备到此槽。
	 * @param Item The item instance to check. 要检查的道具实例。
	 * @return True if the item matches the slot's tag query, false otherwise. 如果道具匹配槽的标签查询则返回true，否则返回false。
	 */
	bool MatchItem(const UGIS_ItemInstance* Item) const;
};

/**
 * Structure representing a group of item slots with index-to-slot and slot-to-index mappings.
 * 表示一组道具槽的结构体，包含索引到槽和槽到索引的映射。
 */
USTRUCT()
struct GENERICINVENTORYSYSTEM_API FGIS_ItemSlotGroup
{
	GENERATED_BODY()

	/**
	 * Mapping of indices to slot tags in the group.
	 * 组内索引到槽标签的映射。
	 */
	UPROPERTY(EditAnywhere, Category=ItemSlot)
	TMap<int32, FGameplayTag> IndexToSlotMap;

	/**
	 * Mapping of slot tags to indices in the group.
	 * 组内槽标签到索引的映射。
	 */
	UPROPERTY(EditAnywhere, Category=ItemSlot, meta=(ForceInlineRow))
	TMap<FGameplayTag, int32> SlotToIndexMap;
};
