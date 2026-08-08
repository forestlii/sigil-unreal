// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "Engine/DataAsset.h"
#include "GIS_CoreStructLibray.h"
#include "GIS_InventoryMeesages.h"
#include "Items/GIS_ItemInfo.h"
#include "Items/GIS_ItemStack.h"
#include "GIS_ItemCollection.generated.h"

class UGIS_ItemRestriction;
class UGIS_SerializationFunctionLibrary;
class UGIS_ItemRestrictionSet;
class UGIS_InventorySubsystem;
class UGIS_ItemCollection;
class UGIS_InventorySystemComponent;

/**
 * Delegate triggered when the item collection is updated.
 * 道具集合更新时触发的委托。
 * @param Message The update message containing collection changes. 包含集合变更的更新消息。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGIS_Collection_UpdateSignature, const FGIS_InventoryCollectionUpdateMessage&, Message);

/**
 * Holds static configuration for an item collection.
 * 存储道具集合的静态配置。
 */
UCLASS(BlueprintType, NotBlueprintable)
class GENERICINVENTORYSYSTEM_API UGIS_ItemCollectionDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Checks if the collection definition supports networking.
	 * 检查集合定义是否支持网络。
	 * @return True if networking is supported, false otherwise. 如果支持网络则返回true，否则返回false。
	 */
	virtual bool IsSupportedForNetworking() const override;

	/**
	 * The unique tag of the item collection for querying in the inventory.
	 * 道具集合的唯一标签，用于在库存中查询。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Common", meta=(Categories="GIS.Collection"))
	FGameplayTag CollectionTag;

	/**
	 * Restrictions applied to the collection for complex use cases.
	 * 为复杂用例应用于集合的限制。
	 * @details Restrictions perform pre-checks to determine if items can be added or removed.
	 * @细节 限制会在道具操作前执行检查，以决定是否可以添加或移除道具。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category="Common")
	TArray<TObjectPtr<UGIS_ItemRestriction>> Restrictions;

	/**
	 * Options for handling overflow when an item cannot fit in the collection.
	 * 当道具无法放入集合时处理溢出的选项。
	 */
	UPROPERTY(EditAnywhere, Category="Common")
	FGIS_ItemOverflowOptions OverflowOptions;

	/**
	 * Gets the class for instantiating the collection.
	 * 获取用于实例化集合的类。
	 * @return The collection instance class. 集合实例类。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Common")
	virtual TSubclassOf<UGIS_ItemCollection> GetCollectionInstanceClass() const;
};

/**
 * Base class for a normal item collection.
 * 常规道具集合的基类。
 */
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew, CollapseCategories, DisplayName="GIS Collection (Normal)")
class GENERICINVENTORYSYSTEM_API UGIS_ItemCollection : public UObject
{
	GENERATED_BODY()

	friend UGIS_ItemCollectionDefinition;
	friend UGIS_InventorySystemComponent;
	friend FGIS_ItemStackContainer;

public:
	/**
	 * Helper struct to lock notifications during collection updates.
	 * 用于在集合更新期间锁定通知的辅助结构体。
	 */
	struct GIS_CollectionNotifyLocker
	{
		/**
		 * Constructor that locks notifications for the collection.
		 * 为集合锁定通知的构造函数。
		 * @param InItemCollection The collection to lock notifications for. 要锁定通知的集合。
		 */
		GIS_CollectionNotifyLocker(UGIS_ItemCollection& InItemCollection);

		/**
		 * Destructor that unlocks notifications.
		 * 解锁通知的析构函数。
		 */
		~GIS_CollectionNotifyLocker();

		/**
		 * The collection being managed.
		 * 被管理的集合。
		 */
		UGIS_ItemCollection& ItemCollection;
	};

	/**
	 * Constructor for the item collection.
	 * 道具集合的构造函数。
	 * @param ObjectInitializer The object initializer. 对象初始化器。
	 */
	UGIS_ItemCollection(const FObjectInitializer& ObjectInitializer);

	//~UObject interface
	/**
	 * Gets the properties that should be replicated for this object.
	 * 获取需要为此对象复制的属性。
	 * @param OutLifetimeProps Array to store the replicated properties. 存储复制属性的数组。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Calls a remote function on the object.
	 * 在对象上调用远程函数。
	 * @param Function The function to call. 要调用的函数。
	 * @param Parms The function parameters. 函数参数。
	 * @param OutParms The output parameters. 输出参数。
	 * @param Stack The function call stack. 函数调用堆栈。
	 * @return True if the function call was successful, false otherwise. 如果函数调用成功则返回true，否则返回false。
	 */
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;

	/**
	 * Gets the function call space for the object.
	 * 获取对象的函数调用空间。
	 * @param Function The function to query. 要查询的函数。
	 * @param Stack The function call stack. 函数调用堆栈。
	 * @return The call space identifier. 调用空间标识符。
	 */
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

	/**
	 * Checks if the object supports networking.
	 * 检查对象是否支持网络。
	 * @return True if networking is supported, false otherwise. 如果支持网络则返回true，否则返回false。
	 */
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	/**
	 * Checks if the collection is initialized.
	 * 检查集合是否已初始化。
	 * @return True if the collection is initialized, false otherwise. 如果集合已初始化则返回true，否则返回false。
	 */
	bool IsInitialized() const;

	/**
	 * Gets the inventory that owns this collection.
	 * 获取拥有此集合的库存。
	 * @return The owning inventory, or nullptr if not set. 所属库存，如果未设置则返回nullptr。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ItemCollection")
	UGIS_InventorySystemComponent* GetOwningInventory() const { return OwningInventory; };

	/**
	 * Gets the unique tag of this collection.
	 * 获取此集合的唯一标签。
	 * @return The collection tag. 集合标签。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|ItemCollection")
	FGameplayTag GetCollectionTag() const { return CollectionTag; };

	/**
	 * Gets the unique ID of this collection.
	 * 获取此集合的唯一ID。
	 * @return The collection ID. 集合ID。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|ItemCollection")
	FGuid GetCollectionId() const { return CollectionId; };

	/**
	 * Gets the definition from which this collection was created.
	 * 获取此集合的源定义。
	 * @return The collection definition, or nullptr if not set. 集合定义，如果未设置则返回nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|ItemCollection")
	const UGIS_ItemCollectionDefinition* GetDefinition() const { return Definition; };

	/**
	 * Gets the display name of the collection.
	 * 获取集合的显示名称。
	 * @return The display name. 显示名称。
	 */
	FString GetCollectionName() const;

	/**
	 * Gets a debug string representation of the collection.
	 * 获取集合的调试字符串表示。
	 * @return The debug string. 调试字符串。
	 */
	FString GetDebugString() const;

#pragma region HasRegion
	/**
	 * Checks if the collection contains a specified item.
	 * 检查集合是否包含指定道具。
	 * @param Item The item instance to check. 要检查的道具实例。
	 * @param Amount The amount to check for. 要检查的数量。
	 * @param SimilarItem Whether to check for similar items or exact matches. 是否检查相似道具或精确匹配。
	 * @return True if the collection contains at least the specified amount, false otherwise. 如果集合包含至少指定数量则返回true，否则返回false。
	 */
	virtual bool HasItem(const UGIS_ItemInstance* Item, int32 Amount, bool SimilarItem = true) const;

#pragma endregion HasRegion

#pragma region Add&Remove
	/**
	 * Adds an item to the collection.
	 * 将道具添加到集合。
	 * @param ItemInfo The item information to add. 要添加的道具信息。
	 * @return The item that was not added or empty if added successfully. 未添加的道具，如果成功添加则为空。
	 */
	virtual FGIS_ItemInfo AddItem(const FGIS_ItemInfo& ItemInfo);

	/**
	 * Adds multiple items to the collection.
	 * 将多个道具添加到集合。
	 * @param ItemInfos The array of item information to add. 要添加的道具信息数组。
	 * @return The number of items added. 添加的道具数量。
	 */
	virtual int32 AddItems(const TArray<FGIS_ItemInfo>& ItemInfos);

	/**
	 * Adds a specific amount of an item to the collection.
	 * 将指定数量的道具添加到集合。
	 * @param Item The item instance to add. 要添加的道具实例。
	 * @param Amount The amount to add. 要添加的数量。
	 * @return The item that was actually added. 实际添加的道具。
	 */
	FGIS_ItemInfo AddItem(UGIS_ItemInstance* Item, int32 Amount);

	/**
	 * Checks if an item can be added to the collection.
	 * 检查道具是否可以添加到集合。
	 * @param InItemInfo The item information to check. 要检查的道具信息。
	 * @param OutItemInfo The item information that can be added (output). 可添加的道具信息（输出）。
	 * @return True if at least one item can be added, false otherwise. 如果至少可以添加一个道具则返回true，否则返回false。
	 */
	virtual bool CanAddItem(const FGIS_ItemInfo& InItemInfo, FGIS_ItemInfo& OutItemInfo);

	/**
	 * Checks if an item can be stacked with an existing stack.
	 * 检查道具是否可以与现有栈堆叠。
	 * @param ItemInfo The item information to check. 要检查的道具信息。
	 * @param ItemStack The item stack to check against. 要检查的道具栈。
	 * @return True if the item can be stacked, false otherwise. 如果道具可以堆叠则返回true，否则返回false。
	 */
	virtual bool CanItemStack(const FGIS_ItemInfo& ItemInfo, const FGIS_ItemStack& ItemStack) const;

	/**
	 * Checks conditions for removing an item from the collection.
	 * 检查从集合移除道具的条件。
	 * @param ItemInfo The item information to remove. 要移除的道具信息。
	 * @param OutItemInfo The item information that can be removed (output). 可移除的道具信息（输出）。
	 * @return True if the item can be removed, false otherwise. 如果道具可以移除则返回true，否则返回false。
	 */
	virtual bool RemoveItemCondition(const FGIS_ItemInfo& ItemInfo, FGIS_ItemInfo& OutItemInfo);

	/**
	 * Removes an item from the collection.
	 * 从集合中移除道具。
	 * @param ItemInfo The item information to remove. 要移除的道具信息。
	 * @return The item that was removed, or empty if nothing was removed. 移除的道具，如果未移除则为空。
	 */
	virtual FGIS_ItemInfo RemoveItem(const FGIS_ItemInfo& ItemInfo);

	/**
	 * Removes all items from the collection.
	 * 从集合中移除所有道具。
	 */
	virtual void RemoveAll();

#pragma endregion Add&Remove

#pragma region GetReference
	/**
	 * Gets item information by stack ID.
	 * 通过栈ID获取道具信息。
	 * @param InStackId The stack ID to query. 要查询的栈ID。
	 * @param OutItemInfo The item information (output). 道具信息（输出）。
	 * @return True if item information was found, false otherwise. 如果找到道具信息则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|ItemCollection")
	bool GetItemInfoByStackId(FGuid InStackId, FGIS_ItemInfo& OutItemInfo) const;

	/**
	 * Finds item information by stack ID.
	 * 通过栈ID查找道具信息。
	 * @param InStackId The stack ID to query. 要查询的栈ID。
	 * @param OutItemInfo The item information (output). 道具信息（输出）。
	 * @return True if item information was found, false otherwise. 如果找到道具信息则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GIS|ItemCollection", meta=(ExpandBoolAsExecs="ReturnValue"))
	bool FindItemInfoByStackId(FGuid InStackId, FGIS_ItemInfo& OutItemInfo) const;

	/**
	 * Retrieves information about a specific item instance.
	 * 获取指定道具实例的信息。
	 * @param Item The item instance to query. 要查询的道具实例。
	 * @param OutItemInfo The item information (output). 道具信息（输出）。
	 * @return True if information was found, false otherwise. 如果找到信息则返回true，否则返回false。
	 */
	virtual bool GetItemInfo(const UGIS_ItemInstance* Item, FGIS_ItemInfo& OutItemInfo) const;

	/**
	 * Gets item information by item definition.
	 * 通过道具定义获取道具信息。
	 * @param ItemDefinition The item definition to query. 要查询的道具定义。
	 * @param OutItemInfo The item information (output). 道具信息（输出）。
	 * @return True if information was found, false otherwise. 如果找到信息则返回true，否则返回false。
	 */
	bool GetItemInfoByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition>& ItemDefinition, FGIS_ItemInfo& OutItemInfo);

	/**
	 * Gets all item information for a specific item definition.
	 * 获取指定道具定义的所有道具信息。
	 * @param ItemDefinition The item definition to query. 要查询的道具定义。
	 * @param OutItemInfos The array of item information (output). 道具信息数组（输出）。
	 * @return True if information was found, false otherwise. 如果找到信息则返回true，否则返回false。
	 */
	bool GetItemInfosByDefinition(const TSoftObjectPtr<UGIS_ItemDefinition>& ItemDefinition, TArray<FGIS_ItemInfo>& OutItemInfos);

	/**
	 * Gets the amount of a specific item in the collection.
	 * 获取集合中指定道具的数量。
	 * @param Item The item instance to query. 要查询的道具实例。
	 * @param SimilarItem Whether to count similar items or exact matches. 是否计数相似道具或精确匹配。
	 * @return The amount of the item in the collection. 集合中的道具数量。
	 */
	int32 GetItemAmount(const UGIS_ItemInstance* Item, bool SimilarItem = true) const;

	/**
	 * Gets the amount of items with a specific definition in the collection.
	 * 获取集合中具有指定定义的道具数量。
	 * @param ItemDefinition The item definition to query. 要查询的道具定义。
	 * @param CountStacks Whether to count the number of stacks instead of total amount. 是否计数栈数量而不是总数量。
	 * @return The number of items or stacks in the collection. 集合中的道具或栈数量。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|ItemCollection")
	int32 GetItemAmount(TSoftObjectPtr<UGIS_ItemDefinition> ItemDefinition, bool CountStacks = false) const;

	/**
	 * Gets all item information in the collection.
	 * 获取集合中的所有道具信息。
	 * @return Array of item information. 道具信息数组。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|ItemCollection")
	TArray<FGIS_ItemInfo> GetAllItemInfos() const;

	/**
	 * Gets all item instances in the collection.
	 * 获取集合中的所有道具实例。
	 * @return Array of item instances. 道具实例数组。
	 */
	TArray<UGIS_ItemInstance*> GetAllItems() const;

#pragma endregion GetReference

#pragma region Giver
	/**
	 * Gives an item to another collection.
	 * 将道具给予另一个集合。
	 * @param ItemInfo The item information to give. 要给予的道具信息。
	 * @param ItemCollection The target collection to receive the item. 接收道具的目标集合。
	 * @return The item information that was given. 给予的道具信息。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GIS|ItemCollection")
	virtual FGIS_ItemInfo GiveItem(const FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ItemCollection);

	/**
	 * Server function to give an item to another collection.
	 * 服务器函数，将道具给予另一个集合。
	 * @param ItemInfo The item information to give. 要给予的道具信息。
	 * @param ItemCollection The target collection to receive the item. 接收道具的目标集合。
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="GIS|ItemCollection")
	void ServerGiveItem(const FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ItemCollection);

	/**
	 * Implementation of ServerGiveItem.
	 * ServerGiveItem 的实现。
	 * @param ItemInfo The item information to give. 要给予的道具信息。
	 * @param ItemCollection The target collection to receive the item. 接收道具的目标集合。
	 */
	virtual void ServerGiveItem_Implementation(const FGIS_ItemInfo& ItemInfo, UGIS_ItemCollection* ItemCollection);

	/**
	 * Gives all items in this collection to another collection.
	 * 将此集合中的所有道具给予另一个集合。
	 * @param OtherItemCollection The target collection to receive the items. 接收道具的目标集合。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GIS|ItemCollection")
	virtual void GiveAllItems(UGIS_ItemCollection* OtherItemCollection);

	/**
	 * Server function to give all items to another collection.
	 * 服务器函数，将所有道具给予另一个集合。
	 * @param OtherItemCollection The target collection to receive the items. 接收道具的目标集合。
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="GIS|ItemCollection")
	void ServerGiveAllItems(UGIS_ItemCollection* OtherItemCollection);

	/**
	 * Calculates how many items can fit given a limited number of additional stacks.
	 * 计算在有限额外栈数下可以容纳的道具数量。
	 * @param ItemInfo The item information to check. 要检查的道具信息。
	 * @param AvailableAdditionalStacks The number of additional stacks allowed. 允许的额外栈数。
	 * @return The number of items that can fit. 可容纳的道具数量。
	 */
	virtual int32 GetItemAmountFittingInLimitedAdditionalStacks(const FGIS_ItemInfo& ItemInfo, int32 AvailableAdditionalStacks) const;

#pragma endregion Giver

	/**
	 * Event triggered when the item collection is updated.
	 * 道具集合更新时触发的事件。
	 */
	UPROPERTY(BlueprintAssignable)
	FGIS_Collection_UpdateSignature OnItemCollectionUpdate;

#pragma region ItemStacks
	/**
	 * Gets all item stacks in the collection.
	 * 获取集合中的所有道具栈。
	 * @return Array of item stacks. 道具栈数组。
	 */
	const TArray<FGIS_ItemStack>& GetAllItemStacks() const;

	/**
	 * Gets the number of item stacks in the collection.
	 * 获取集合中的道具栈数量。
	 * @return The number of item stacks. 道具栈数量。
	 */
	int32 GetItemStacksNum() const;

protected:
	/**
	 * Adds an item stack to the collection.
	 * 将道具栈添加到集合。
	 * @param Stack The item stack to add. 要添加的道具栈。
	 */
	virtual void AddItemStack(const FGIS_ItemStack& Stack);

	/**
	 * Removes an item stack at a specific index.
	 * 在指定索引移除道具栈。
	 * @param Idx The index of the stack to remove. 要移除的栈索引。
	 * @param bRemoveFromCollection Whether to remove the item instance from the collection. 是否从集合中移除道具实例。
	 */
	virtual void RemoveItemStackAtIndex(int32 Idx, bool bRemoveFromCollection = true);

	/**
	 * Updates the amount of an item stack at a specific index.
	 * 更新指定索引的道具栈数量。
	 * @param Idx The index of the stack to update. 要更新的栈索引。
	 * @param NewAmount The new amount for the stack. 栈的新数量。
	 */
	virtual void UpdateItemStackAmountAtIndex(int32 Idx, int32 NewAmount);

	/**
	 * Called before an item stack is added (server-side only).
	 * 在添加道具栈之前调用（仅限服务器端）。
	 * @param Stack The item stack to add. 要添加的道具栈。
	 * @param Idx The index where the stack will be added. 栈将添加的索引。
	 */
	virtual void OnPreItemStackAdded(const FGIS_ItemStack& Stack, int32 Idx);

	/**
	 * Called when an item stack is added (client and server).
	 * 道具栈添加时调用（客户端和服务器）。
	 * @param Stack The added item stack. 添加的道具栈。
	 */
	virtual void OnItemStackAdded(const FGIS_ItemStack& Stack);

	/**
	 * Called when an item stack is removed (client and server).
	 * 道具栈移除时调用（客户端和服务器）。
	 * @param Stack The removed item stack. 移除的道具栈。
	 */
	virtual void OnItemStackRemoved(const FGIS_ItemStack& Stack);

	/**
	 * Called when an item stack is updated (client and server).
	 * 道具栈更新时调用（客户端和服务器）。
	 * @param Stack The updated item stack. 更新的道具栈。
	 */
	virtual void OnItemStackUpdated(const FGIS_ItemStack& Stack);

	/**
	 * Processes pending item stacks.
	 * 处理待处理的道具栈。
	 */
	void ProcessPendingItemStacks();

	/**
	 * Store the stack id and position mapping within this collection.
	 * 存储在此集合中Stack id到位置的映射关系。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="ItemCollection", Transient, meta=(ForceInlineRow))
	TMap<FGuid, int32> StackToIdxMap;

	/**
	 * Store the stack id and position mapping within this collection.
	 * 存储在此集合中Stack id到位置的映射关系。
	 */
	// UPROPERTY(VisibleInstanceOnly, Category="ItemCollection", Transient)
	// TMap<FGuid, int32> IdxToStackMap;

private:
	/**
	 * Temporary storage for pending item stacks.
	 * 待处理道具栈的临时存储。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="ItemCollection", Transient)
	TMap<FGuid, FGIS_ItemStack> PendingItemStacks;


#pragma endregion

protected:
	/**
	 * Internal function to add an item to the collection.
	 * 内部函数，将道具添加到集合。
	 * @param ItemInfo The item information to add. 要添加的道具信息。
	 * @return The item that was actually added. 实际添加的道具。
	 */
	virtual FGIS_ItemInfo AddInternal(const FGIS_ItemInfo& ItemInfo);

	/**
	 * Handles overflow when an item cannot be fully added.
	 * 处理道具无法完全添加时的溢出。
	 * @param OriginalItemInfo The original item information. 原始道具信息。
	 * @param ItemInfoAdded The item information that was added. 已添加的道具信息。
	 */
	virtual void HandleItemOverflow(const FGIS_ItemInfo& OriginalItemInfo, const FGIS_ItemInfo& ItemInfoAdded);

	/**
	 * Internal function to remove an item from the collection.
	 * 内部函数，从集合中移除道具。
	 * @param ItemInfo The item information to remove. 要移除的道具信息。
	 * @return The item that was removed. 移除的道具。
	 */
	virtual FGIS_ItemInfo RemoveInternal(const FGIS_ItemInfo& ItemInfo);

	/**
	 * Simplifies internal item removal logic.
	 * 简化内部道具移除逻辑。
	 * @param ItemInfo The item information to remove. 要移除的道具信息。
	 * @param AlreadyRemoved The amount already removed (modified). 已移除的数量（可修改）。
	 * @param StackIndex The stack index to remove from. 要移除的栈索引。
	 * @return The item stack was actually removed. 实际被移除的道具栈。
	 */
	virtual FGIS_ItemStack SimpleInternalItemRemove(const FGIS_ItemInfo& ItemInfo, int32& AlreadyRemoved, int32 StackIndex);

	/**
	 * Indicates whether the collection is initialized.
	 * 指示集合是否已初始化。
	 */
	UPROPERTY(Transient)
	bool bInitialized = false;

	/**
	 * Counter for locking notifications.
	 * 用于锁定通知的计数器。
	 */
	int32 NotifyLocker = 0;

	/**
	 * Container for item stacks.
	 * 道具栈的容器。
	 */
	UPROPERTY(VisibleInstanceOnly, Category="ItemCollection", Replicated, meta=(ShowOnlyInnerProperties))
	FGIS_ItemStackContainer Container;

	/**
	 * The unique tag of the item collection for querying in the inventory.
	 * 道具集合的唯一标签，用于在库存中查询。
	 */
	UPROPERTY(Transient)
	FGameplayTag CollectionTag;

	/**
	 * The collection definition.
	 * 集合定义。
	 */
	UPROPERTY(Transient)
	TObjectPtr<const UGIS_ItemCollectionDefinition> Definition{nullptr};

	/**
	 * The unique ID of the collection.
	 * 集合的唯一ID。
	 */
	UPROPERTY(Transient)
	FGuid CollectionId;

	/**
	 * The inventory that owns this collection.
	 * 拥有此集合的库存。
	 */
	UPROPERTY(Transient)
	TObjectPtr<UGIS_InventorySystemComponent> OwningInventory;

	/**
	 * Sets the collection definition.
	 * 设置集合定义。
	 * @param NewDefinition The new collection definition. 新的集合定义。
	 */
	virtual void SetDefinition(const UGIS_ItemCollectionDefinition* NewDefinition);

	/**
	 * Sets the unique tag of the collection.
	 * 设置集合的唯一标签。
	 * @param NewTag The new collection tag. 新的集合标签。
	 */
	void SetCollectionTag(FGameplayTag NewTag);

	/**
	 * Sets the unique ID of the collection.
	 * 设置集合的唯一ID。
	 * @param NewId The new collection ID. 新的集合ID。
	 */
	void SetCollectionId(FGuid NewId);

	/**
	 * Sets the owning inventory for the collection.
	 * 设置集合的所属库存。
	 * @param NewInventory The new owning inventory. 新的所属库存。
	 */
	void SetInventory(UGIS_InventorySystemComponent* NewInventory) { OwningInventory = NewInventory; };
};
