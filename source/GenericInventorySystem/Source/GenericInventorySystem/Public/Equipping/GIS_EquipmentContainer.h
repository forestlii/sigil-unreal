// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GIS_EquipmentContainer.generated.h"

class UGIS_ItemInstance;
class UGIS_EquipmentInstance;
class UGIS_ItemDefinition;
class UGIS_EquipmentSystemComponent;
struct FGIS_EquipmentContainer;

/**
 * Structure representing an equipment entry in the container.
 * 表示容器中装备条目的结构体。
 */
USTRUCT()
struct GENERICINVENTORYSYSTEM_API FGIS_EquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/**
	 * Default constructor for equipment entry.
	 * 装备条目的默认构造函数。
	 */
	FGIS_EquipmentEntry()
	{
	}

	/**
	 * Gets a debug string representation of the equipment entry.
	 * 获取装备条目的调试字符串表示。
	 * @return The debug string. 调试字符串。
	 */
	FString GetDebugString() const;

private:
	friend FGIS_EquipmentContainer;
	friend UGIS_EquipmentSystemComponent;

	/**
	 * The equipment instance.
	 * 装备实例。
	 */
	UPROPERTY(VisibleAnywhere, Category="Equipment", meta=(ShowInnerProperties))
	TObjectPtr<UObject> Instance = nullptr;

	/**
	 * The item instance associated with this equipment.
	 * 与此装备关联的道具实例。
	 */
	UPROPERTY(VisibleAnywhere, Category="Equipment")
	TObjectPtr<UGIS_ItemInstance> ItemInstance{nullptr};

	/**
	 * The slot where the equipment is equipped.
	 * 装备所在的装备槽。
	 */
	UPROPERTY(VisibleAnywhere, Category="Equipment")
	FGameplayTag EquippedSlot;

	/**
	 * Indicates whether the equipment is active.
	 * 指示装备是否处于激活状态。
	 */
	UPROPERTY(VisibleAnywhere, Category="Equipment")
	bool bActive{false};

	/**
	 * Previous active state (not replicated).
	 * 上一个激活状态（不复制）。
	 */
	UPROPERTY(NotReplicated)
	bool bPrevActive = false;

	/**
	 * Checks if the equipment entry is valid.
	 * 检查装备条目是否有效。
	 * @return True if the entry is valid, false otherwise. 如果条目有效则返回true，否则返回false。
	 */
	bool IsValid() const;
};

/**
 * Container for a list of applied equipment.
 * 存储已应用装备列表的容器。
 */
USTRUCT()
struct GENERICINVENTORYSYSTEM_API FGIS_EquipmentContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	/**
	 * Default constructor for equipment container.
	 * 装备容器的默认构造函数。
	 */
	FGIS_EquipmentContainer()
		: OwningComponent(nullptr)
	{
	}

	/**
	 * Constructor for equipment container with an owning component.
	 * 使用所属组件构造装备容器。
	 * @param InComponent The owning equipment system component. 所属的装备系统组件。
	 */
	FGIS_EquipmentContainer(UGIS_EquipmentSystemComponent* InComponent)
		: OwningComponent(InComponent)
	{
	}

	//~FFastArraySerializer contract
	/**
	 * Called before equipment entries are removed during replication.
	 * 复制期间在移除装备条目前调用。
	 * @param RemovedIndices The indices of removed entries. 移除条目的索引。
	 * @param FinalSize The final size of the array after removal. 移除后数组的最终大小。
	 */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	/**
	 * Called after equipment entries are added during replication.
	 * 复制期间在添加装备条目后调用。
	 * @param AddedIndices The indices of added entries. 添加条目的索引。
	 * @param FinalSize The final size of the array after addition. 添加后数组的最终大小。
	 */
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	/**
	 * Called after equipment entries are changed during replication.
	 * 复制期间在装备条目更改后调用。
	 * @param ChangedIndices The indices of changed entries. 更改条目的索引。
	 * @param FinalSize The final size of the array after changes. 更改后数组的最终大小。
	 */
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	/**
	 * Handles delta serialization for the equipment container.
	 * 处理装备容器的增量序列化。
	 * @param DeltaParms The serialization parameters. 序列化参数。
	 * @return True if serialization was successful, false otherwise. 如果序列化成功则返回true，否则返回false。
	 */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FGIS_EquipmentEntry, FGIS_EquipmentContainer>(Entries, DeltaParms, *this);
	}


	int32 IndexOfBySlot(const FGameplayTag& Slot) const;

	int32 IndexOfByItem(const UGIS_ItemInstance* Item) const;

	int32 IndexOfByItemId(const FGuid& ItemId) const;

	/**
	 * Replicated list of equipment entries.
	 * 复制的装备条目列表。
	 */
	UPROPERTY(VisibleAnywhere, Category="EquipmentSystem", meta=(ShowOnlyInnerProperties, DisplayName="Equipments"))
	TArray<FGIS_EquipmentEntry> Entries;

	/**
	 * The equipment system component that owns this container.
	 * 拥有此容器的装备系统组件。
	 */
	UPROPERTY()
	TObjectPtr<UGIS_EquipmentSystemComponent> OwningComponent;
};

/**
 * Template specialization to enable network delta serialization for the equipment container.
 * 为装备容器启用网络增量序列化的模板特化。
 */
template <>
struct TStructOpsTypeTraits<FGIS_EquipmentContainer> : TStructOpsTypeTraitsBase2<FGIS_EquipmentContainer>
{
	enum { WithNetDeltaSerializer = true };
};
