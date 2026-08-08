// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/WorldSubsystem.h"
#include "SigilGameUIExtensionSubsystem.generated.h"

class USigilExtensionSubsystem;
struct FSigilGameUIExtRequest;
template <typename T>
class TSubclassOf;
template <typename T>
class TSoftClassPtr;
class FSubsystemCollectionBase;
class UUserWidget;
struct FFrame;

/**
 * Enum defining match rules for UI extension points.
 * 定义UI扩展点匹配规则的枚举。
 */
UENUM(BlueprintType)
enum class ESigilGameUIExtPointMatchType : uint8
{
	/**
	 * Requires an exact match for the extension point tag.
	 * 要求扩展点标签完全匹配。
	 */
	ExactMatch,

	/**
	 * Allows partial matches for the extension point tag.
	 * 允许扩展点标签部分匹配。
	 */
	PartialMatch
};

/**
 * Enum defining actions for UI extensions.
 * 定义UI扩展动作的枚举。
 */
UENUM(BlueprintType)
enum class ESigilGameUIExtAction : uint8
{
	/**
	 * Extension is added.
	 * 扩展被添加。
	 */
	Added,

	/**
	 * Extension is removed.
	 * 扩展被移除。
	 */
	Removed
};

/**
 * Delegate for extension point events.
 * 扩展点事件的委托。
 */
DECLARE_DELEGATE_TwoParams(FExtendExtensionPointDelegate, ESigilGameUIExtAction Action, const FSigilGameUIExtRequest& Request);

/**
 * Structure representing a UI extension.
 * 表示UI扩展的结构。
 */
struct FSigilGameUIExt : TSharedFromThis<FSigilGameUIExt>
{
	/**
	 * Tag identifying the extension point.
	 * 标识扩展点的标签。
	 */
	FGameplayTag ExtensionPointTag;

	/**
	 * Priority of the extension.
	 * 扩展的优先级。
	 */
	int32 Priority = INDEX_NONE;

	/**
	 * Context object for the extension.
	 * 扩展的上下文对象。
	 */
	TWeakObjectPtr<UObject> ContextObject;

	/**
	 * Data object for the extension, kept alive by the subsystem.
	 * 扩展的数据对象，由子系统保持存活。
	 */
	TObjectPtr<UObject> Data = nullptr;
};

/**
 * Structure representing a UI extension point.
 * 表示UI扩展点的结构。
 */
struct FSigilGameUIExtPoint : TSharedFromThis<FSigilGameUIExtPoint>
{
	/**
	 * Tag identifying the extension point.
	 * 标识扩展点的标签。
	 */
	FGameplayTag ExtensionPointTag;

	/**
	 * Context object for the extension point.
	 * 扩展点的上下文对象。
	 */
	TWeakObjectPtr<UObject> ContextObject;

	/**
	 * Match type for the extension point tag.
	 * 扩展点标签的匹配类型。
	 */
	ESigilGameUIExtPointMatchType ExtensionPointTagMatchType = ESigilGameUIExtPointMatchType::ExactMatch;

	/**
	 * Allowed data classes for the extension point.
	 * 扩展点允许的数据类。
	 */
	TArray<TObjectPtr<UClass>> AllowedDataClasses;

	/**
	 * Callback for extension point events.
	 * 扩展点事件的回调。
	 */
	FExtendExtensionPointDelegate Callback;

	/**
	 * Checks if an extension matches the extension point.
	 * 检查扩展是否与扩展点匹配。
	 * @param Extension The extension to check. 要检查的扩展。
	 * @return True if the extension matches, false otherwise. 如果扩展匹配返回true，否则返回false。
	 */
	bool DoesExtensionPassContract(const FSigilGameUIExt* Extension) const;
};

/**
 * Handle for a UI extension point.
 * UI扩展点的句柄。
 */
USTRUCT(BlueprintType)
struct SIGILUI_API FSigilGameUIExtPointHandle
{
	GENERATED_BODY()

	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	FSigilGameUIExtPointHandle();

	/**
	 * Unregisters the extension point.
	 * 注销扩展点。
	 */
	void Unregister();

	/**
	 * Checks if the handle is valid.
	 * 检查句柄是否有效。
	 * @return True if valid, false otherwise. 如果有效返回true，否则返回false。
	 */
	bool IsValid() const { return DataPtr.IsValid(); }

	/**
	 * Equality operator for extension point handles.
	 * 扩展点句柄的相等比较运算符。
	 */
	bool operator==(const FSigilGameUIExtPointHandle& Other) const { return DataPtr == Other.DataPtr; }

	/**
	 * Inequality operator for extension point handles.
	 * 扩展点句柄的不等比较运算符。
	 */
	bool operator!=(const FSigilGameUIExtPointHandle& Other) const { return !operator==(Other); }

	/**
	 * Hash function for the extension point handle.
	 * 扩展点句柄的哈希函数。
	 * @param Handle The handle to hash. 要哈希的句柄。
	 * @return The hash value. 哈希值。
	 */
	friend uint32 GetTypeHash(const FSigilGameUIExtPointHandle& Handle)
	{
		return PointerHash(Handle.DataPtr.Get());
	};

private:
	/**
	 * The extension subsystem source.
	 * 扩展子系统源。
	 */
	TWeakObjectPtr<USigilExtensionSubsystem> ExtensionSource;

	/**
	 * Shared pointer to the extension point data.
	 * 扩展点数据的共享指针。
	 */
	TSharedPtr<FSigilGameUIExtPoint> DataPtr;

	friend USigilExtensionSubsystem;

	/**
	 * Constructor for the extension point handle.
	 * 扩展点句柄构造函数。
	 * @param InExtensionSource The extension subsystem source. 扩展子系统源。
	 * @param InDataPtr The extension point data. 扩展点数据。
	 */
	FSigilGameUIExtPointHandle(USigilExtensionSubsystem* InExtensionSource, const TSharedPtr<FSigilGameUIExtPoint>& InDataPtr);
};

template <>
struct TStructOpsTypeTraits<FSigilGameUIExtPointHandle> : TStructOpsTypeTraitsBase2<FSigilGameUIExtPointHandle>
{
	enum
	{
		WithCopy = true,
		WithIdenticalViaEquality = true,
	};
};

/**
 * Handle for a UI extension.
 * UI扩展的句柄。
 */
USTRUCT(BlueprintType)
struct SIGILUI_API FSigilGameUIExtHandle
{
	GENERATED_BODY()

	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	FSigilGameUIExtHandle();

	/**
	 * Unregisters the extension.
	 * 注销扩展。
	 */
	void Unregister();

	/**
	 * Checks if the handle is valid.
	 * 检查句柄是否有效。
	 * @return True if valid, false otherwise. 如果有效返回true，否则返回false。
	 */
	bool IsValid() const { return DataPtr.IsValid(); }

	/**
	 * Equality operator for extension handles.
	 * 扩展句柄的相等比较运算符。
	 */
	bool operator==(const FSigilGameUIExtHandle& Other) const { return DataPtr == Other.DataPtr; }

	/**
	 * Inequality operator for extension handles.
	 * 扩展句柄的不等比较运算符。
	 */
	bool operator!=(const FSigilGameUIExtHandle& Other) const { return !operator==(Other); }

	/**
	 * Hash function for the extension handle.
	 * 扩展句柄的哈希函数。
	 * @param Handle The handle to hash. 要哈希的句柄。
	 * @return The hash value. 哈希值。
	 */
	friend uint32 GetTypeHash(FSigilGameUIExtHandle Handle) { return PointerHash(Handle.DataPtr.Get()); };

private:
	/**
	 * The extension subsystem source.
	 * 扩展子系统源。
	 */
	TWeakObjectPtr<USigilExtensionSubsystem> ExtensionSource;

	/**
	 * Shared pointer to the extension data.
	 * 扩展数据的共享指针。
	 */
	TSharedPtr<FSigilGameUIExt> DataPtr;

	friend USigilExtensionSubsystem;

	/**
	 * Constructor for the extension handle.
	 * 扩展句柄构造函数。
	 * @param InExtensionSource The extension subsystem source. 扩展子系统源。
	 * @param InDataPtr The extension data. 扩展数据。
	 */
	FSigilGameUIExtHandle(USigilExtensionSubsystem* InExtensionSource, const TSharedPtr<FSigilGameUIExt>& InDataPtr);
};

template <>
struct TStructOpsTypeTraits<FSigilGameUIExtHandle> : TStructOpsTypeTraitsBase2<FSigilGameUIExtHandle>
{
	enum
	{
		WithCopy = true,
		WithIdenticalViaEquality = true,
	};
};

/**
 * Structure representing a UI extension request.
 * 表示UI扩展请求的结构。
 */
USTRUCT(BlueprintType)
struct FSigilGameUIExtRequest
{
	GENERATED_BODY()

	/**
	 * Handle for the extension.
	 * 扩展的句柄。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GUIS")
	FSigilGameUIExtHandle ExtensionHandle;

	/**
	 * Tag identifying the extension point.
	 * 标识扩展点的标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GUIS")
	FGameplayTag ExtensionPointTag;

	/**
	 * Priority of the extension.
	 * 扩展的优先级。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GUIS")
	int32 Priority = INDEX_NONE;

	/**
	 * Data object for the extension.
	 * 扩展的数据对象。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GUIS")
	TObjectPtr<UObject> Data = nullptr;

	/**
	 * Context object for the extension.
	 * 扩展的上下文对象。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GUIS")
	TObjectPtr<UObject> ContextObject = nullptr;
};

/**
 * Dynamic delegate for extension point events.
 * 扩展点事件的动态委托。
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FExtendExtensionPointDynamicDelegate, ESigilGameUIExtAction, Action, const FSigilGameUIExtRequest&, ExtensionRequest);

/**
 * World subsystem for managing UI extensions.
 * 管理UI扩展的世界子系统。
 */
UCLASS()
class SIGILUI_API USigilExtensionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Registers an extension point.
	 * 注册扩展点。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param ExtensionPointTagMatchType The match type for the tag. 标签匹配类型。
	 * @param AllowedDataClasses The allowed data classes. 允许的数据类。
	 * @param ExtensionCallback The callback for extension events. 扩展事件回调。
	 * @return The extension point handle. 扩展点句柄。
	 */
	FSigilGameUIExtPointHandle RegisterExtensionPoint(const FGameplayTag& ExtensionPointTag, ESigilGameUIExtPointMatchType ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses,
	                                                  FExtendExtensionPointDelegate ExtensionCallback);

	/**
	 * Registers an extension point for a specific context.
	 * 为特定上下文注册扩展点。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param ContextObject The context object. 上下文对象。
	 * @param ExtensionPointTagMatchType The match type for the tag. 标签匹配类型。
	 * @param AllowedDataClasses The allowed data classes. 允许的数据类。
	 * @param ExtensionCallback The callback for extension events. 扩展事件回调。
	 * @return The extension point handle. 扩展点句柄。
	 */
	FSigilGameUIExtPointHandle RegisterExtensionPointForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, ESigilGameUIExtPointMatchType ExtensionPointTagMatchType,
	                                                            const TArray<UClass*>& AllowedDataClasses, FExtendExtensionPointDelegate ExtensionCallback);

	/**
	 * Registers a widget as a UI extension.
	 * 将小部件注册为UI扩展。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param WidgetClass The widget class. 小部件类。
	 * @param Priority The extension priority. 扩展优先级。
	 * @return The extension handle. 扩展句柄。
	 */
	FSigilGameUIExtHandle RegisterExtensionAsWidget(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);

	/**
	 * Registers a widget as a UI extension for a specific context.
	 * 为特定上下文将小部件注册为UI扩展。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param ContextObject The context object. 上下文对象。
	 * @param WidgetClass The widget class. 小部件类。
	 * @param Priority The extension priority. 扩展优先级。
	 * @return The extension handle. 扩展句柄。
	 */
	FSigilGameUIExtHandle RegisterExtensionAsWidgetForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);

	/**
	 * Registers data as a UI extension.
	 * 将数据注册为UI扩展。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param ContextObject The context object. 上下文对象。
	 * @param Data The data object. 数据对象。
	 * @param Priority The extension priority. 扩展优先级。
	 * @return The extension handle. 扩展句柄。
	 */
	FSigilGameUIExtHandle RegisterExtensionAsData(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority);

	/**
	 * Unregisters a UI extension.
	 * 注销UI扩展。
	 * @param ExtensionHandle The extension handle. 扩展句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	void UnregisterExtension(const FSigilGameUIExtHandle& ExtensionHandle);

	/**
	 * Unregisters a UI extension point.
	 * 注销UI扩展点。
	 * @param ExtensionPointHandle The extension point handle. 扩展点句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	void UnregisterExtensionPoint(const FSigilGameUIExtPointHandle& ExtensionPointHandle);

	/**
	 * Adds referenced objects to the garbage collector.
	 * 将引用的对象添加到垃圾回收器。
	 * @param InThis The subsystem instance. 子系统实例。
	 * @param Collector The reference collector. 引用收集器。
	 */
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

protected:
	/**
	 * Initializes the subsystem.
	 * 初始化子系统。
	 * @param Collection The subsystem collection. 子系统集合。
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Deinitializes the subsystem.
	 * 反初始化子系统。
	 */
	virtual void Deinitialize() override;

	/**
	 * Notifies an extension point of new or removed extensions.
	 * 通知扩展点有关新添加或移除的扩展。
	 * @param ExtensionPoint The extension point to notify. 要通知的扩展点。
	 */
	void NotifyExtensionPointOfExtensions(TSharedPtr<FSigilGameUIExtPoint>& ExtensionPoint);

	/**
	 * Notifies extension points of an extension action.
	 * 通知扩展点有关扩展动作。
	 * @param Action The extension action (Added/Removed). 扩展动作（添加/移除）。
	 * @param Extension The extension data. 扩展数据。
	 */
	void NotifyExtensionPointsOfExtension(ESigilGameUIExtAction Action, TSharedPtr<FSigilGameUIExt>& Extension);

	/**
	 * Registers an extension point (Blueprint version).
	 * 注册扩展点（蓝图版本）。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param ExtensionPointTagMatchType The match type for the tag. 标签匹配类型。
	 * @param AllowedDataClasses The allowed data classes. 允许的数据类。
	 * @param ExtensionCallback The callback for extension events. 扩展事件回调。
	 * @return The extension point handle. 扩展点句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension Point"))
	FSigilGameUIExtPointHandle K2_RegisterExtensionPoint(FGameplayTag ExtensionPointTag, ESigilGameUIExtPointMatchType ExtensionPointTagMatchType,
	                                                     const TArray<TSoftClassPtr<UClass>>& AllowedDataClasses,
	                                                     FExtendExtensionPointDynamicDelegate ExtensionCallback);

	/**
	 * Registers a widget as a UI extension (Blueprint version).
	 * 将小部件注册为UI扩展（蓝图版本）。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param WidgetClass The widget class. 小部件类。
	 * @param Priority The extension priority. 扩展优先级。
	 * @return The extension handle. 扩展句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget)"))
	FSigilGameUIExtHandle K2_RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSoftClassPtr<UUserWidget> WidgetClass, int32 Priority = -1);

	/**
	 * Registers a widget as a UI extension for a specific context (Blueprint version).
	 * 为特定上下文将小部件注册为UI扩展（蓝图版本）。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param WidgetClass The widget class. 小部件类。
	 * @param ContextObject The context object. 上下文对象。
	 * @param Priority The extension priority. 扩展优先级。
	 * @return The extension handle. 扩展句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget For Context)"))
	FSigilGameUIExtHandle K2_RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSoftClassPtr<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority = -1);

	/**
	 * Registers data as a UI extension (Blueprint version).
	 * 将数据注册为UI扩展（蓝图版本）。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param Data The data object. 数据对象。
	 * @param Priority The extension priority. 扩展优先级。
	 * @return The extension handle. 扩展句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data)"))
	FSigilGameUIExtHandle K2_RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority = -1);

	/**
	 * Registers data as a UI extension for a specific context (Blueprint version).
	 * 为特定上下文将数据注册为UI扩展（蓝图版本）。
	 * @param ExtensionPointTag The extension point tag. 扩展点标签。
	 * @param ContextObject The context object. 上下文对象。
	 * @param Data The data object. 数据对象。
	 * @param Priority The extension priority. 扩展优先级。
	 * @return The extension handle. 扩展句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data For Context)"))
	FSigilGameUIExtHandle K2_RegisterExtensionAsDataForContext(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority = -1);

	/**
	 * Creates an extension request from extension data.
	 * 从扩展数据创建扩展请求。
	 * @param Extension The extension data. 扩展数据。
	 * @return The extension request. 扩展请求。
	 */
	FSigilGameUIExtRequest CreateExtensionRequest(const TSharedPtr<FSigilGameUIExt>& Extension);

private:
	/**
	 * Map of extension point tags to extension point lists.
	 * 扩展点标签到扩展点列表的映射。
	 */
	using FExtensionPointList = TArray<TSharedPtr<FSigilGameUIExtPoint>>;
	TMap<FGameplayTag, FExtensionPointList> ExtensionPointMap;

	/**
	 * Map of extension tags to extension lists.
	 * 扩展标签到扩展列表的映射。
	 */
	using FExtensionList = TArray<TSharedPtr<FSigilGameUIExt>>;
	TMap<FGameplayTag, FExtensionList> ExtensionMap;
};

/**
 * Blueprint function library for UI extension operations.
 * UI扩展操作的蓝图函数库。
 */
UCLASS()
class SIGILUI_API USigilExtensionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	USigilExtensionFunctionLibrary();

	/**
	 * Unregisters a UI extension.
	 * 注销UI扩展。
	 * @param Handle The extension handle. 扩展句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static void UnregisterExtension(UPARAM(ref)
		FSigilGameUIExtHandle& Handle);

	/**
	 * Checks if a UI extension is valid.
	 * 检查UI扩展是否有效。
	 * @param Handle The extension handle. 扩展句柄。
	 * @return True if valid, false otherwise. 如果有效返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "UI Extension")
	static bool IsValidExtension(UPARAM(ref)
		FSigilGameUIExtHandle& Handle);

	/**
	 * Unregisters a UI extension point.
	 * 注销UI扩展点。
	 * @param Handle The extension point handle. 扩展点句柄。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static void UnregisterExtensionPoint(UPARAM(ref)
		FSigilGameUIExtPointHandle& Handle);

	/**
	 * Checks if a UI extension point is valid.
	 * 检查UI扩展点是否有效。
	 * @param Handle The extension point handle. 扩展点句柄。
	 * @return True if valid, false otherwise. 如果有效返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category = "UI Extension")
	static bool IsValidExtensionPoint(UPARAM(ref)
		FSigilGameUIExtPointHandle& Handle);
};
