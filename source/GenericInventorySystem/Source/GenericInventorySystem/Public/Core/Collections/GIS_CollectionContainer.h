// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GIS_CollectionContainer.generated.h"

class UGIS_ItemCollectionDefinition;
class UGIS_InventorySystemComponent;
class UGIS_ItemCollection;
struct FGIS_CollectionContainer;

/**
 * Structure representing an entry in the collection container.
 * 表示集合容器中条目的结构体。
 */
USTRUCT()
struct GENERICINVENTORYSYSTEM_API FGIS_CollectionEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/**
	 * Default constructor for collection entry.
	 * 集合条目的默认构造函数。
	 */
	FGIS_CollectionEntry() : Instance(nullptr)
	{
	}

	/**
	 * Unique ID of the collection entry.
	 * 集合条目的唯一ID。
	 */
	UPROPERTY(VisibleAnywhere, Category="GIS")
	FGuid Id = FGuid();

	/**
	 * The collection definition associated with this entry.
	 * 与此条目关联的集合定义。
	 */
	UPROPERTY(VisibleAnywhere, Category="GIS")
	TObjectPtr<const UGIS_ItemCollectionDefinition> Definition;

	/**
	 * The collection instance associated with this entry.
	 * 与此条目关联的集合实例。
	 */
	UPROPERTY(VisibleAnywhere, Category="GIS", meta=(ShowInnerProperties))
	TObjectPtr<UGIS_ItemCollection> Instance = nullptr;

	/**
	 * Checks if the collection entry is valid.
	 * 检查集合条目是否有效。
	 * @return True if the entry is valid, false otherwise. 如果条目有效则返回true，否则返回false。
	 */
	bool IsValidEntry() const;
};

/**
 * Container for storing a list of item collections with replication support.
 * 用于存储道具集合列表的容器，支持复制。
 */
USTRUCT()
struct GENERICINVENTORYSYSTEM_API FGIS_CollectionContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	/**
	 * Default constructor for collection container.
	 * 集合容器的默认构造函数。
	 */
	FGIS_CollectionContainer()
		: OwningComponent(nullptr)
	{
	}

	/**
	 * Constructor for collection container with an owning inventory component.
	 * 使用所属库存组件构造集合容器。
	 * @param InInventory The owning inventory system component. 所属的库存系统组件。
	 */
	FGIS_CollectionContainer(UGIS_InventorySystemComponent* InInventory);

	//~FFastArraySerializer contract
	/**
	 * Called before collection entries are removed during replication.
	 * 复制期间在移除集合条目前调用。
	 * @param RemovedIndices The indices of removed entries. 移除条目的索引。
	 * @param FinalSize The final size of the array after removal. 移除后数组的最终大小。
	 */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	/**
	 * Called after collection entries are added during replication.
	 * 复制期间在添加集合条目后调用。
	 * @param AddedIndices The indices of added entries. 添加条目的索引。
	 * @param FinalSize The final size of the array after addition. 添加后数组的最终大小。
	 */
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	/**
	 * Called after collection entries are changed during replication.
	 * 复制期间在集合条目更改后调用。
	 * @param ChangedIndices The indices of changed entries. 更改条目的索引。
	 * @param FinalSize The final size of the array after changes. 更改后数组的最终大小。
	 */
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	/**
	 * Handles delta serialization for the collection container.
	 * 处理集合容器的增量序列化。
	 * @param DeltaParms The serialization parameters. 序列化参数。
	 * @return True if serialization was successful, false otherwise. 如果序列化成功则返回true，否则返回false。
	 */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FGIS_CollectionEntry, FGIS_CollectionContainer>(Entries, DeltaParms, *this);
	}

	/**
	 * Replicated list of collection entries.
	 * 复制的集合条目列表。
	 */
	UPROPERTY(VisibleAnywhere, Category="InventorySystem", meta=(ShowOnlyInnerProperties, DisplayName="Collections"))
	TArray<FGIS_CollectionEntry> Entries;

	/**
	 * The inventory system component that owns this container.
	 * 拥有此容器的库存系统组件。
	 */
	UPROPERTY(Transient)
	TObjectPtr<UGIS_InventorySystemComponent> OwningComponent;
};

/**
 * Template specialization to enable network delta serialization for the collection container.
 * 为集合容器启用网络增量序列化的模板特化。
 */
template <>
struct TStructOpsTypeTraits<FGIS_CollectionContainer> : TStructOpsTypeTraitsBase2<FGIS_CollectionContainer>
{
	enum { WithNetDeltaSerializer = true };
};
