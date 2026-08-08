// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"

#include "SigilGameUIFunctionLibrary.generated.h"

class USigilGameUILayout;
enum class ECommonInputType : uint8;
template <typename T>
class TSubclassOf;

class APlayerController;
class UCommonActivatableWidget;
class ULocalPlayer;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayTag;

/**
 * Common functions for Game UI.
 * 游戏UI的通用功能函数库。
 */
UCLASS()
class SIGILUI_API USigilGameUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	USigilGameUIFunctionLibrary()
	{
	}

	/**
	 * Gets the input type of the owning player for the specified widget.
	 * 获取指定控件所属玩家的输入类型。
	 * @param WidgetContextObject The widget to query for input type. 要查询输入类型的控件。
	 * @return The input type of the owning player. 所属玩家的输入类型。
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WidgetContextObject"))
	static ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	/**
	 * Checks if the owning player is using touch input.
	 * 检查所属玩家是否使用触摸输入。
	 * @param WidgetContextObject The widget to query for input type. 要查询输入类型的控件。
	 * @return True if the owning player is using touch input, false otherwise. 如果所属玩家使用触摸输入则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	/**
	 * Checks if the owning player is using a gamepad.
	 * 检查所属玩家是否使用游戏手柄。
	 * @param WidgetContextObject The widget to query for input type. 要查询输入类型的控件。
	 * @return True if the owning player is using a gamepad, false otherwise. 如果所属玩家使用游戏手柄则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	/**
	 * Pushes a widget to the specified UI layer for the given player.
	 * 将控件推送到指定玩家的UI层。
	 * @param PlayerController The player controller to associate with the UI layer. 与UI层关联的玩家控制器。
	 * @param LayerName The tag identifying the UI layer. 标识UI层的标签。
	 * @param WidgetClass The class of the widget to push. 要推送的控件类。
	 * @return The created widget instance. 创建的控件实例。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta=(DynamicOutputParam="ReturnValue", DeterminesOutputType="WidgetClass"))
	static UCommonActivatableWidget* PushContentToUILayer_ForPlayer(const APlayerController* PlayerController, UPARAM(meta = (Categories = "Sigil.UI.Layer"))
	                                                                FGameplayTag LayerName,
	                                                                UPARAM(meta = (AllowAbstract = false))
	                                                                TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/**
	 * Pops content from the specified UI layer for the given player.
	 * 从指定玩家的UI层中弹出内容。
	 * @param PlayerController The player controller associated with the UI layer. 与UI层关联的玩家控制器。
	 * @param LayerName The tag identifying the UI layer. 标识UI层的标签。
	 * @param RemainNum Number of widgets to remain in the layer (-1 means remove all.). 保留在层中的控件数量（-1表示不保留）。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS")
	static void PopContentFromUILayer_ForPlayer(const APlayerController* PlayerController, UPARAM(meta = (Categories = "Sigil.UI.Layer"))
	                                            FGameplayTag LayerName, int32 RemainNum = -1);

	/**
	 * Removes a specific activatable widget from the UI layer.
	 * 从UI层中移除指定的可激活控件。
	 * @param ActivatableWidget The widget to remove. 要移除的控件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta=(DefaultToSelf="ActivatableWidget"))
	static void PopContentFromUILayer(UCommonActivatableWidget* ActivatableWidget);

	/**
	 * Pops multiple activatable widgets from the UI layer.
	 * 从UI层中弹出多个可激活控件。
	 * @param ActivatableWidgets List of activatable widgets to pop. 要弹出的控件列表。
	 * @param bReverse If true, pops in reverse array order. 如果为true，则按数组反向顺序弹出。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta=(DefaultToSelf="ActivatableWidget"))
	static void PopContentsFromUILayer(TArray<UCommonActivatableWidget*> ActivatableWidgets, bool bReverse = true);

	/**
	 * Gets the local player associated with the given player controller.
	 * 获取与指定玩家控制器关联的本地玩家。
	 * @param PlayerController The player controller to query. 要查询的玩家控制器。
	 * @return The associated local player. 关联的本地玩家。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintCosmetic, Category="GUIS")
	static ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

	/**
	 * Gets the game UI layout for the specified player.
	 * 获取指定玩家的游戏UI布局。
	 * @param PlayerController The player controller to query. 要查询的玩家控制器。
	 * @return The game UI layout for the player. 玩家的游戏UI布局。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS")
	static USigilGameUILayout* GetGameUILayoutForPlayer(const APlayerController* PlayerController);

	/**
	 * Suspends input for the specified player with a reason.
	 * 暂停指定玩家的输入并提供原因。
	 * @param PlayerController The player controller to suspend input for. 要暂停输入的玩家控制器。
	 * @param SuspendReason The reason for suspending input. 暂停输入的原因。
	 * @return A token representing the suspension. 表示暂停的令牌。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS")
	static FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);

	/**
	 * Suspends input for the specified local player with a reason.
	 * 暂停指定本地玩家的输入并提供原因。
	 * @param LocalPlayer The local player to suspend input for. 要暂停输入的本地玩家。
	 * @param SuspendReason The reason for suspending input. 暂停输入的原因。
	 * @return A token representing the suspension. 表示暂停的令牌。
	 */
	static FName SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason);

	/**
	 * Resumes input for the specified player using the suspension token.
	 * 使用暂停令牌恢复指定玩家的输入。
	 * @param PlayerController The player controller to resume input for. 要恢复输入的玩家控制器。
	 * @param SuspendToken The token from the suspension call. 暂停调用时返回的令牌。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS")
	static void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);

	/**
	 * Resumes input for the specified local player using the suspension token.
	 * 使用暂停令牌恢复指定本地玩家的输入。
	 * @param LocalPlayer The local player to resume input for. 要恢复输入的本地玩家。
	 * @param SuspendToken The token from the suspension call. 暂停调用时返回的令牌。
	 */
	static void ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken);

	/**
	 * Returns the typed item in the owning list view that this entry is currently assigned to represent.
	 * 以指定类型的方式获取Entry要展示的Item。
	 * @param UserObjectListEntry The list entry to query (defaults to self). 要查询的列表条目（默认值为自身）。
	 * @param DesiredClass The type of ListItemObject. 列表项对象的类型。
	 * @return The typed item object. 类型化的列表项对象。
	 */
	UFUNCTION(BlueprintPure, Category="GUIS",
		meta = (DeterminesOutputType = "DesiredClass", DynamicOutputParam = "ReturnValue", DefaultToSelf = UserObjectListEntry, DisplayName = "Get List item Object"))
	static UObject* GetTypedListItem(TScriptInterface<IUserObjectListEntry> UserObjectListEntry, TSubclassOf<UObject> DesiredClass);

	/**
	 * Safely returns the typed item in the owning list view that this entry is currently assigned to represent.
	 * 安全地以指定类型的方式获取Entry要展示的Item。
	 * @param UserObjectListEntry The list entry to query (defaults to self). 要查询的列表条目（默认值为自身）。
	 * @param DesiredClass The type of ListItemObject. 列表项对象的类型。
	 * @param OutItem The typed item object (output). 类型化的列表项对象（输出）。
	 * @return True if the item was successfully retrieved, false otherwise. 如果成功获取项则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS",
		meta = (ExpandBoolAsExecs = "ReturnValue", DeterminesOutputType = "DesiredClass", DynamicOutputParam = "OutItemObject", DefaultToSelf = UserObjectListEntry, DisplayName =
			"Get List item Object"))
	static bool GetTypedListItemSafely(TScriptInterface<IUserObjectListEntry> UserObjectListEntry, TSubclassOf<UObject> DesiredClass, UObject*& OutItem);

private:
	/**
	 * Tracks the number of active input suspensions.
	 * 跟踪当前活动的输入暂停数量。
	 */
	static int32 InputSuspensions;
};
