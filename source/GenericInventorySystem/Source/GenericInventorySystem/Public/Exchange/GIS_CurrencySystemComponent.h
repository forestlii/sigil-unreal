// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIS_CurrencyContainer.h"
#include "GIS_CurrencyDefinition.h"
#include "Components/ActorComponent.h"
#include "GIS_CurrencySystemComponent.generated.h"

class UGIS_InventorySubsystem;

/**
 * Delegate triggered when a currency amount changes.
 * 货币数量变化时触发的委托。
 * @param Currency The currency definition that changed. 发生变化的货币定义。
 * @param OldAmount The previous amount of the currency. 之前的货币数量。
 * @param NewAmount The new amount of the currency. 新的货币数量。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGIS_CurrencyChangedSignature, const UGIS_CurrencyDefinition*, Currency, float, OldAmount, float, NewAmount);

/**
 * Currency system component for managing an actor's currencies.
 * 管理演员货币的货币系统组件。
 * @details This component serves as a wrapper for the CurrencyContainer, handling currency-related operations.
 * @细节 该组件作为CurrencyContainer的包装器，处理与货币相关的操作。
 */
UCLASS(ClassGroup=(GIS), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class GENERICINVENTORYSYSTEM_API UGIS_CurrencySystemComponent : public UActorComponent
{
	GENERATED_BODY()

	friend FGIS_CurrencyContainer;

public:
	/**
	 * Constructor for the currency system component.
	 * 货币系统组件的构造函数。
	 */
	UGIS_CurrencySystemComponent();

	/**
	 * Gets the currency system component from an actor.
	 * 从一个演员获取货币系统组件。
	 * @param Actor The actor to query for the currency system component. 要查询货币系统组件的演员。
	 * @return The currency system component, or nullptr if not found. 货币系统组件，如果未找到则返回nullptr。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GIS|Inventory", Meta = (DefaultToSelf="Actor"))
	static UGIS_CurrencySystemComponent* GetCurrencySystemComponent(const AActor* Actor);

	/**
	 * Gets the properties that should be replicated for this component.
	 * 获取需要为此组件复制的属性。
	 * @param OutLifetimeProps Array to store the replicated properties. 存储复制属性的数组。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Initializes the component.
	 * 初始化组件。
	 */
	virtual void InitializeComponent() override;

	/**
	 * Gets all currency information.
	 * 获取所有货币信息。
	 * @return Array of currency entries containing currency types and amounts. 包含货币类型和数量的货币条目数组。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual TArray<FGIS_CurrencyEntry> GetAllCurrencies() const;

	/**
	 * Sets all currency information.
	 * 设置所有货币信息。
	 * @param InCurrencyInfos The array of currency entries to set. 要设置的货币条目数组。
	 */
	virtual void SetCurrencies(const TArray<FGIS_CurrencyEntry>& InCurrencyInfos);

	/**
	 * Clears all currency information.
	 * 清空所有货币信息。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual void EmptyCurrencies();

	/**
	 * Gets information for a specific currency.
	 * 获取指定货币的信息。
	 * @param CurrencyDefinition The currency definition to query. 要查询的货币定义。
	 * @param OutCurrencyInfo The currency information (output). 货币信息（输出）。
	 * @return True if the currency is found, false otherwise. 如果找到货币则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GIS|CurrencySystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	virtual bool GetCurrency(TSoftObjectPtr<const UGIS_CurrencyDefinition> CurrencyDefinition, FGIS_CurrencyEntry& OutCurrencyInfo) const;

	/**
	 * Gets information for multiple specified currencies.
	 * 获取多个指定货币的信息。
	 * @param CurrencyDefinitions The array of currency definitions to query. 要查询的货币定义数组。
	 * @param OutCurrencyInfos The array of currency information (output). 货币信息数组（输出）。
	 * @return True if all specified currencies are found, false otherwise. 如果找到所有指定货币则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GIS|CurrencySystem", meta=(ExpandBoolAsExecs="ReturnValue"))
	virtual bool GetCurrencies(TArray<TSoftObjectPtr<UGIS_CurrencyDefinition>> CurrencyDefinitions, TArray<FGIS_CurrencyEntry>& OutCurrencyInfos) const;

	/**
	 * Adds a currency and its amount.
	 * 添加货币及其数量。
	 * @param CurrencyInfo The currency information to add. 要添加的货币信息。
	 * @return True if the currency was added successfully, false otherwise. 如果货币添加成功则返回true，否则返回false。
	 * @details Adds the specified amount to an existing currency or creates a new entry if the currency doesn't exist.
	 * @细节 如果当前没有该货币，则以指定数量新增货币；如果已有，则在原始数量上累加。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual bool AddCurrency(FGIS_CurrencyEntry CurrencyInfo);

	/**
	 * Removes a currency and its amount.
	 * 移除指定数量的货币。
	 * @param CurrencyInfo The currency information to remove. 要移除的货币信息。
	 * @return True if the currency was removed successfully, false if the currency doesn't exist. 如果货币移除成功则返回true，如果货币不存在则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual bool RemoveCurrency(FGIS_CurrencyEntry CurrencyInfo);

	/**
	 * Checks if the specified currency amount is available.
	 * 检查是否拥有指定数量的货币。
	 * @param CurrencyInfo The currency information to check. 要检查的货币信息。
	 * @return True if the specified amount is available, false otherwise. 如果有足够数量的货币则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual bool HasCurrency(FGIS_CurrencyEntry CurrencyInfo) const;

	/**
	 * Checks if multiple specified currency amounts are available.
	 * 检查是否拥有多个指定数量的货币。
	 * @param CurrencyInfos The array of currency information to check. 要检查的货币信息数组。
	 * @return True if all specified amounts are available, false otherwise. 如果所有货币数量都满足则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual bool HasCurrencies(const TArray<FGIS_CurrencyEntry>& CurrencyInfos);

	/**
	 * Adds multiple currencies.
	 * 添加多个货币。
	 * @param CurrencyInfos The array of currency information to add. 要添加的货币信息数组。
	 * @return True if all currencies were added successfully, false otherwise. 如果所有货币都添加成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual bool AddCurrencies(const TArray<FGIS_CurrencyEntry>& CurrencyInfos);

	/**
	 * Removes multiple currencies.
	 * 移除多个货币。
	 * @param CurrencyInfos The array of currency information to remove. 要移除的货币信息数组。
	 * @return True if all currencies were removed successfully, false otherwise. 如果所有货币都移除成功则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GIS|CurrencySystem")
	virtual bool RemoveCurrencies(const TArray<FGIS_CurrencyEntry>& CurrencyInfos);

	/**
	 * Called when the actor begins play.
	 * 演员开始播放时调用。
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when the actor ends play.
	 * 演员结束播放时调用。
	 * @param EndPlayReason The reason for ending play. 结束播放的原因。
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Event triggered when a currency amount changes.
	 * 货币数量变化时触发的事件。
	 */
	UPROPERTY(BlueprintAssignable)
	FGIS_CurrencyChangedSignature OnCurrencyChangedEvent;

#pragma region Static Calculations
#if 0
	/**
	 * Finds the index of a currency in a list.
	 * 查找包含指定货币的索引。
	 * @param Currency The currency definition to find. 要查找的货币定义。
	 * @param List The list of currency entries to search. 要搜索的货币条目列表。
	 * @return The index of the currency, or -1 if not found. 货币的索引，如果未找到则返回-1。
	 */
	static int32 FindIndexWithCurrency(const UGIS_CurrencyDefinition* Currency, const TArray<FGIS_CurrencyEntry>& List);

	/**
	 * Finds or creates an index for a currency in a list.
	 * 查找或创建包含指定货币的索引。
	 * @param Currency The currency definition to find or create. 要查找或创建的货币定义。
	 * @param Result The list of currency entries (modified). 货币条目列表（可修改）。
	 * @return The index of the currency. 货币的索引。
	 */
	static int32 FindOrCreateCurrencyIndex(const UGIS_CurrencyDefinition* Currency, TArray<FGIS_CurrencyEntry>& Result);

	/**
	 * Performs discrete addition of two currency lists.
	 * 执行两个货币列表的离散加法。
	 * @param Lhs The first currency list. 第一个货币列表。
	 * @param Rhs The second currency list. 第二个货币列表。
	 * @return The combined currency list. 合并后的货币列表。
	 */
	static TArray<FGIS_CurrencyEntry> DiscreteAddition(const TArray<FGIS_CurrencyEntry>& Lhs, const TArray<FGIS_CurrencyEntry>& Rhs);

	/**
	 * Adds a single currency and amount to a currency list.
	 * 将单一货币和数量添加到货币列表。
	 * @param CurrencyAmounts The currency list to modify. 要修改的货币列表。
	 * @param Currency The currency definition to add. 要添加的货币定义。
	 * @param Amount The amount to add. 要添加的数量。
	 * @return The updated currency list. 更新后的货币列表。
	 */
	static TArray<FGIS_CurrencyEntry> DiscreteAddition(const TArray<FGIS_CurrencyEntry>& CurrencyAmounts,
	                                                   const UGIS_CurrencyDefinition* Currency, float Amount);

	/**
	 * Sets the fractional part of a currency to its maximum value.
	 * 将货币的小数部分设置为最大值。
	 * @param CurrencyAmounts The currency list to modify. 要修改的货币列表。
	 * @param Currency The currency definition to adjust. 要调整的货币定义。
	 * @return The updated currency list. 更新后的货币列表。
	 */
	static TArray<FGIS_CurrencyEntry> SetAllFractionToMax(const TArray<FGIS_CurrencyEntry>& CurrencyAmounts, const UGIS_CurrencyDefinition* Currency);

	/**
	 * Converts an amount to discrete currency units.
	 * 将数量转换为离散的货币单位。
	 * @param Currency The currency definition to convert. 要转换的货币定义。
	 * @param Amount The amount to convert. 要转换的数量。
	 * @return The discrete currency entries. 离散的货币条目。
	 */
	static TArray<FGIS_CurrencyEntry> ConvertToDiscrete(const UGIS_CurrencyDefinition* Currency, double Amount);

	/**
	 * Converts overflow amounts, ignoring fractional parts.
	 * 转换溢出金额，忽略小数部分。
	 * @param Currency The currency definition to convert. 要转换的货币定义。
	 * @param Amount The amount to convert. 要转换的数量。
	 * @return The overflow currency entries. 溢出的货币条目。
	 */
	static TArray<FGIS_CurrencyEntry> ConvertOverflow(const UGIS_CurrencyDefinition* Currency, double Amount);

	/**
	 * Converts the fractional part of an amount to currency units.
	 * 将金额的小数部分转换为货币单位。
	 * @param Currency The currency definition to convert. 要转换的货币定义。
	 * @param Amount The amount to convert. 要转换的数量。
	 * @return The fractional currency entries. 小数部分的货币条目。
	 */
	static TArray<FGIS_CurrencyEntry> ConvertFraction(const UGIS_CurrencyDefinition* Currency, double Amount);

	/**
	 * Gets the maximum amount for a currency in discrete units.
	 * 获取货币的最大金额（以离散单位计）。
	 * @param Currency The currency definition to query. 要查询的货币定义。
	 * @return The maximum currency entries. 最大货币条目。
	 */
	static TArray<FGIS_CurrencyEntry> MaxedOutAmount(const UGIS_CurrencyDefinition* Currency);
#endif
#pragma endregion

protected:
	/**
	 * Called when a currency entry is added.
	 * 货币条目添加时调用。
	 * @param Entry The added currency entry. 添加的货币条目。
	 * @param Idx The index of the added entry. 添加条目的索引。
	 */
	virtual void OnCurrencyEntryAdded(const FGIS_CurrencyEntry& Entry, int32 Idx);

	/**
	 * Called when a currency entry is removed.
	 * 货币条目移除时调用。
	 * @param Entry The removed currency entry. 移除的货币条目。
	 * @param Idx The index of the removed entry. 移除条目的索引。
	 */
	virtual void OnCurrencyEntryRemoved(const FGIS_CurrencyEntry& Entry, int32 Idx);

	/**
	 * Called when a currency entry is updated.
	 * 货币条目更新时调用。
	 * @param Entry The updated currency entry. 更新的货币条目。
	 * @param Idx The index of the updated entry. 更新条目的索引。
	 * @param OldAmount The previous amount of the currency. 之前的货币数量。
	 * @param NewAmount The new amount of the currency. 新的货币数量。
	 */
	virtual void OnCurrencyEntryUpdated(const FGIS_CurrencyEntry& Entry, int32 Idx, float OldAmount, float NewAmount);

	/**
	 * Called when a currency amount changes.
	 * 货币数量变化时调用。
	 * @param Tag The currency definition that changed. 发生变化的货币定义。
	 * @param OldValue The previous amount of the currency. 之前的货币数量。
	 * @param NewValue The new amount of the currency. 新的货币数量。
	 */
	virtual void OnCurrencyChanged(TObjectPtr<const UGIS_CurrencyDefinition> Tag, float OldValue, float NewValue);

	/**
	 * Adds initial currencies to the component.
	 * 向组件添加初始货币。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GIS|CurrencySystem")
	void AddInitialCurrencies();

	/**
	 * Internal function to get information for a specific currency.
	 * 获取指定货币信息的内部函数。
	 * @param CurrencyTag The currency definition to query. 要查询的货币定义。
	 * @param OutCurrencyInfo The currency information (output). 货币信息（输出）。
	 * @return True if the currency is found, false otherwise. 如果找到货币则返回true，否则返回false。
	 */
	virtual bool GetCurrencyInternal(const TObjectPtr<const UGIS_CurrencyDefinition>& CurrencyTag, FGIS_CurrencyEntry& OutCurrencyInfo) const;

	/**
	 * Internal function to get information for multiple currencies.
	 * 获取多个货币信息的内部函数。
	 * @param Currencies The array of currency definitions to query. 要查询的货币定义数组。
	 * @param OutCurrencies The array of currency information (output). 货币信息数组（输出）。
	 * @return True if all currencies are found, false otherwise. 如果找到所有货币则返回true，否则返回false。
	 */
	virtual bool GetCurrenciesInternal(const TArray<TObjectPtr<const UGIS_CurrencyDefinition>>& Currencies, TArray<FGIS_CurrencyEntry>& OutCurrencies) const;

	/**
	 * Internal function to add a currency.
	 * 添加货币的内部函数。
	 * @param CurrencyInfo The currency information to add. 要添加的货币信息。
	 * @param bNotify Whether to trigger notifications for the addition. 是否触发添加通知。
	 * @return True if the currency was added successfully, false otherwise. 如果货币添加成功则返回true，否则返回false。
	 */
	virtual bool AddCurrencyInternal(const FGIS_CurrencyEntry& CurrencyInfo, bool bNotify = true);

	/**
	 * Internal function to remove a currency.
	 * 移除货币的内部函数。
	 * @param CurrencyInfo The currency information to remove. 要移除的货币信息。
	 * @param bNotify Whether to trigger notifications for the removal. 是否触发移除通知。
	 * @return True if the currency was removed successfully, false otherwise. 如果货币移除成功则返回true，否则返回false。
	 */
	virtual bool RemoveCurrencyInternal(const FGIS_CurrencyEntry& CurrencyInfo, bool bNotify = true);

	/**
	 * Internal function to check if a currency amount is available.
	 * 检查货币数量是否可用的内部函数。
	 * @param CurrencyInfo The currency information to check. 要检查的货币信息。
	 * @return True if the specified amount is available, false otherwise. 如果有足够数量的货币则返回true，否则返回false。
	 */
	virtual bool HasCurrencyInternal(const FGIS_CurrencyEntry& CurrencyInfo) const;

	/**
	 * Default currencies to initialize the component with.
	 * 初始化组件的默认货币。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Currency")
	TArray<FGIS_CurrencyEntry> DefaultCurrencies;

	/**
	 * Container for currency entries.
	 * 货币条目的容器。
	 */
	UPROPERTY(VisibleAnywhere, Replicated, Category="Currency", meta=(ShowOnlyInnerProperties))
	FGIS_CurrencyContainer Container;

	/**
	 * Local cache mapping currency definitions to their amounts.
	 * 货币定义到其数量的本地缓存映射。
	 */
	UPROPERTY()
	TMap<TObjectPtr<const UGIS_CurrencyDefinition>, float> CurrencyMap;
};
