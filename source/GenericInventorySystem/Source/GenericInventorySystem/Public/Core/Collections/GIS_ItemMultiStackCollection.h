// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIS_ItemCollection.h"
#include "GIS_ItemMultiStackCollection.generated.h"

class UGIS_ItemMultiStackCollection;

/**
 * Definition for a multi-stack item collection.
 * 多栈道具集合的定义。
 */
UCLASS(BlueprintType)
class UGIS_ItemMultiStackCollectionDefinition : public UGIS_ItemCollectionDefinition
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the multi-stack collection definition.
	 * 多栈集合定义的构造函数。
	 */
	UGIS_ItemMultiStackCollectionDefinition();

	/**
	 * Gets the class for instantiating the collection.
	 * 获取用于实例化集合的类。
	 * @return The collection instance class. 集合实例类。
	 */
	virtual TSubclassOf<UGIS_ItemCollection> GetCollectionInstanceClass() const override;

	/**
	 * Default stack size limit for items without a StackSizeLimitAttribute.
	 * 没有StackSizeLimitAttribute的道具的默认栈大小限制。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StackSettings")
	int32 DefaultStackSizeLimit = 99;

	/**
	 * The integer attribute in the item definition to determine stack size (optional).
	 * 道具定义中用于确定栈大小的整型属性（可选）。
	 * @attention This is optional.
	 * @注意 这是可选的。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StackSettings")
	FGameplayTag StackSizeLimitAttribute;
};

/**
 * An item collection that supports multiple stacks of the same item.
 * 支持相同道具多栈的道具集合。
 */
UCLASS(DisplayName="GIS Item Collection (Multi Stack)")
class GENERICINVENTORYSYSTEM_API UGIS_ItemMultiStackCollection : public UGIS_ItemCollection
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the multi-stack item collection.
	 * 多栈道具集合的构造函数。
	 * @param ObjectInitializer The object initializer. 对象初始化器。
	 */
	UGIS_ItemMultiStackCollection(const FObjectInitializer& ObjectInitializer);

	/**
	 * Retrieves information about an item instance in the collection.
	 * 获取集合中道具实例的信息。
	 * @param Item The item instance to query. 要查询的道具实例。
	 * @param OutItemInfo The item information (output). 道具信息（输出）。
	 * @return True if information was found, false otherwise. 如果找到信息则返回true，否则返回false。
	 */
	virtual bool GetItemInfo(const UGIS_ItemInstance* Item, FGIS_ItemInfo& OutItemInfo) const override;

protected:
	/**
	 * Calculates how many items can fit given a limited number of additional stacks.
	 * 计算在有限额外栈数下可以容纳的道具数量。
	 * @param ItemInfo The item information to check. 要检查的道具信息。
	 * @param AvailableAdditionalStacks The number of additional stacks allowed. 允许的额外栈数。
	 * @return The number of items that can fit. 可容纳的道具数量。
	 */
	virtual int32 GetItemAmountFittingInLimitedAdditionalStacks(const FGIS_ItemInfo& ItemInfo, int32 AvailableAdditionalStacks) const override;

	/**
	 * Internal function to add an item to the collection.
	 * 内部函数，将道具添加到集合。
	 * @param ItemInfo The item information to add. 要添加的道具信息。
	 * @return The item that was actually added. 实际添加的道具。
	 */
	virtual FGIS_ItemInfo AddInternal(const FGIS_ItemInfo& ItemInfo) override;

	/**
	 * Gets the maximum stack size for an item.
	 * 获取道具的最大栈大小。
	 * @param Item The item instance to query. 要查询的道具实例。
	 * @return The maximum stack size for the item. 道具的最大栈大小。
	 */
	int32 GetMaxStackSize(UGIS_ItemInstance* Item) const;

private:
	/**
	 * Internal function to remove an item from the collection.
	 * 内部函数，从集合中移除道具。
	 * @param ItemInfo The item information to remove. 要移除的道具信息。
	 * @return The item that was removed. 移除的道具。
	 */
	virtual FGIS_ItemInfo RemoveInternal(const FGIS_ItemInfo& ItemInfo) override;

	/**
	 * Removes items from a specific stack.
	 * 从指定栈移除道具。
	 * @param Index The index of the stack to remove from. 要移除的栈索引。
	 * @param PrevStackIndexWithSameItem The previous stack index with the same item. 具有相同道具的前一个栈索引。
	 * @param MaxStackSize The maximum stack size. 最大栈大小。
	 * @param AmountToRemove The amount to remove (modified). 要移除的数量（可修改）。
	 * @param AlreadyRemoved The amount already removed (modified). 已移除的数量（可修改）。
	 * @return The stack index with the item. 包含道具的栈索引。
	 */
	int32 RemoveItemFromStack(int32 Index, int32 PrevStackIndexWithSameItem, int32 MaxStackSize, int32& AmountToRemove, int32& AlreadyRemoved);

	/**
	 * Increases the amount in a specific stack.
	 * 增加指定栈中的数量。
	 * @param StackIdx The stack index to increase. 要增加的栈索引。
	 * @param MaxStackSize The maximum stack size. 最大栈大小。
	 * @param AmountToAdd The amount to add (modified). 要添加的数量（可修改）。
	 * @return The amount added to the stack. 添加到栈的数量。
	 */
	int32 IncreaseStackAmount(int32 StackIdx, int32 MaxStackSize, int32& AmountToAdd);
};