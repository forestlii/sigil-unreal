// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "GUIS_GameUIStructLibrary.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GUIS_GameUIPolicy.generated.h"

class UCommonUserWidget;
class UGUIS_GameUIContext;
class ULocalPlayer;
class UGUIS_GameUISubsystem;
class UGUIS_GameUILayout;

/**
 * Enumeration for local multiplayer UI interaction modes.
 * 本地多人UI交互模式的枚举。
 */
UENUM()
enum class EGUIS_LocalMultiplayerInteractionMode : uint8
{
	PrimaryOnly, // Fullscreen viewport for the primary player only. 仅为主玩家显示全屏视口。
	SingleToggle, // Fullscreen viewport with player swapping control. 全屏视口，玩家可切换控制。
	Simultaneous // Simultaneous viewports for all players. 为所有玩家同时显示视口。
};

/**
 * Manages UI layouts for different local players.
 * 为不同本地玩家管理UI布局。
 * @details Controls the creation, addition, and removal of UI layouts and contexts.
 * @细节 控制UI布局和上下文的创建、添加和移除。
 */
UCLASS(Abstract, Blueprintable, Within = GUIS_GameUISubsystem)
class GENERICUISYSTEM_API UGUIS_GameUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the UI policy for the specified world context as a specific type.
	 * 获取指定世界上下文的UI策略，转换为特定类型。
	 * @param WorldContextObject The object providing world context. 提供世界上下文的对象。
	 * @return The UI policy cast to the specified type. 转换为指定类型的UI策略。
	 */
	template <typename GameUIPolicyClass = UGUIS_GameUIPolicy>
	static GameUIPolicyClass* GetGameUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIPolicyClass>(GetGameUIPolicy(WorldContextObject));
	}

	/**
	 * Retrieves the UI policy for the specified world context.
	 * 获取指定世界上下文的UI策略。
	 * @param WorldContextObject The object providing world context. 提供世界上下文的对象。
	 * @return The UI policy. UI策略。
	 */
	static UGUIS_GameUIPolicy* GetGameUIPolicy(const UObject* WorldContextObject);

	/**
	 * Retrieves the world associated with the UI policy.
	 * 获取与UI策略关联的世界。
	 * @return The associated world. 关联的世界。
	 */
	virtual UWorld* GetWorld() const override;

	/**
	 * Retrieves the owning game UI subsystem.
	 * 获取所属的游戏UI子系统。
	 * @return The owning subsystem. 所属子系统。
	 */
	UGUIS_GameUISubsystem* GetOwningSubsystem() const;

	/**
	 * Retrieves the root layout for a local player.
	 * 获取本地玩家的根布局。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @return The root layout for the player. 玩家的根布局。
	 */
	UGUIS_GameUILayout* GetRootLayout(const ULocalPlayer* LocalPlayer) const;

	/**
	 * Retrieves a UI context for a local player.
	 * 获取本地玩家的UI上下文。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param ContextClass The class of the context to retrieve. 要检索的上下文类。
	 * @return The UI context, or nullptr if not found. UI上下文，未找到时为nullptr。
	 */
	virtual UGUIS_GameUIContext* GetContext(const ULocalPlayer* LocalPlayer, TSubclassOf<UGUIS_GameUIContext> ContextClass);

	/**
	 * Adds a UI context for a local player.
	 * 为本地玩家添加UI上下文。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param NewContext The UI context to add. 要添加的UI上下文。
	 * @return True if the context was added, false otherwise. 如果上下文添加成功则返回true，否则返回false。
	 */
	virtual bool AddContext(const ULocalPlayer* LocalPlayer, UGUIS_GameUIContext* NewContext);

	/**
	 * Finds a UI context for a local player.
	 * 查找本地玩家的UI上下文。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param ContextClass The class of the context to find. 要查找的上下文类。
	 * @return The found UI context, or nullptr if not found. 找到的UI上下文，未找到时为nullptr。
	 */
	virtual UGUIS_GameUIContext* FindContext(const ULocalPlayer* LocalPlayer, TSubclassOf<UGUIS_GameUIContext> ContextClass);

	/**
	 * Removes a UI context for a local player.
	 * 移除本地玩家的UI上下文。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param ContextClass The class of the context to remove. 要移除的上下文类。
	 */
	virtual void RemoveContext(const ULocalPlayer* LocalPlayer, TSubclassOf<UGUIS_GameUIContext> ContextClass);

	/**
	 * Adds a UI action binding for a local player.
	 * 为本地玩家添加UI动作绑定。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Target The target widget for the action. 动作的目标控件。
	 * @param InputAction The input action to bind. 要绑定的输入动作。
	 * @param bShouldDisplayInActionBar Whether to display in the action bar. 是否在动作栏中显示。
	 * @param Callback Delegate called when the action is executed. 动作执行时调用的委托。
	 * @param BindingHandle The handle for the binding. 绑定的句柄。
	 */
	virtual void AddUIAction(const ULocalPlayer* LocalPlayer, UCommonUserWidget* Target, const FDataTableRowHandle& InputAction, bool bShouldDisplayInActionBar,
	                         const FGUIS_UIActionExecutedDelegate& Callback,
	                         FGUIS_UIActionBindingHandle& BindingHandle);

	/**
	 * Removes a UI action binding for a local player.
	 * 为本地玩家移除UI动作绑定。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param BindingHandle The handle of the binding to remove. 要移除的绑定句柄。
	 */
	virtual void RemoveUIAction(const ULocalPlayer* LocalPlayer, FGUIS_UIActionBindingHandle& BindingHandle);

	/**
	 * Gets the current multiplayer interaction mode.
	 * 获取当前的多人交互模式。
	 * @return The current interaction mode. 当前交互模式。
	 */
	EGUIS_LocalMultiplayerInteractionMode GetLocalMultiplayerInteractionMode() const { return LocalMultiplayerInteractionMode; }

	/**
	 * Requests primary control for a specific layout.
	 * 为特定布局请求主要控制权。
	 * @param Layout The layout requesting primary control. 请求主要控制权的布局。
	 */
	void RequestPrimaryControl(UGUIS_GameUILayout* Layout);

protected:
	/**
	 * Adds a layout to the viewport for a local player.
	 * 为本地玩家将布局添加到视口。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The layout to add. 要添加的布局。
	 */
	void AddLayoutToViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Removes a layout from the viewport for a local player.
	 * 为本地玩家从视口中移除布局。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The layout to remove. 要移除的布局。
	 */
	void RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Called when a root layout is added to the viewport.
	 * 当根布局添加到视口时调用。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The added layout. 添加的布局。
	 */
	virtual void OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Blueprint event for when a root layout is added to the viewport.
	 * 当根布局添加到视口时的蓝图事件。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The added layout. 添加的布局。
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRootLayoutAddedToViewport"))
	void BP_OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Called when a root layout is removed from the viewport.
	 * 当根布局从视口中移除时调用。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The removed layout. 移除的布局。
	 */
	virtual void OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Blueprint event for when a root layout is removed from the viewport.
	 * 当根布局从视口中移除时的蓝图事件。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The removed layout. 移除的布局。
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRootLayoutRemovedFromViewport"))
	void BP_OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Called when a root layout is released.
	 * 当根布局被释放时调用。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The released layout. 释放的布局。
	 */
	virtual void OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Blueprint event for when a root layout is released.
	 * 当根布局被释放时的蓝图事件。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Layout The released layout. 释放的布局。
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRootLayoutReleased"))
	void BP_OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout);

	/**
	 * Creates a layout widget for a local player.
	 * 为本地玩家创建布局控件。
	 * @param LocalPlayer The local player. 本地玩家。
	 */
	void CreateLayoutWidget(ULocalPlayer* LocalPlayer);

	/**
	 * Retrieves the layout widget class for a local player.
	 * 获取本地玩家的布局控件类。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @return The layout widget class. 布局控件类。
	 */
	TSubclassOf<UGUIS_GameUILayout> GetLayoutWidgetClass(ULocalPlayer* LocalPlayer);

private:
	/**
	 * Current multiplayer interaction mode.
	 * 当前的多人交互模式。
	 */
	EGUIS_LocalMultiplayerInteractionMode LocalMultiplayerInteractionMode = EGUIS_LocalMultiplayerInteractionMode::PrimaryOnly;

	/**
	 * The class used for the game UI layout.
	 * 用于游戏UI布局的类。
	 */
	UPROPERTY(EditAnywhere, Category="GUIS")
	TSoftClassPtr<UGUIS_GameUILayout> LayoutClass;

	/**
	 * Array of root viewport layout information.
	 * 根视口布局信息的数组。
	 */
	UPROPERTY(Transient)
	TArray<FGUIS_RootViewportLayoutInfo> RootViewportLayouts;

	/**
	 * Notifies when a player is added.
	 * 当玩家被添加时通知。
	 * @param LocalPlayer The added local player. 添加的本地玩家。
	 */
	void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);

	/**
	 * Notifies when a player is removed.
	 * 当玩家被移除时通知。
	 * @param LocalPlayer The removed local player. 移除的本地玩家。
	 */
	void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);

	/**
	 * Notifies when a player is destroyed.
	 * 当玩家被销毁时通知。
	 * @param LocalPlayer The destroyed local player. 销毁的本地玩家。
	 */
	void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	friend class UGUIS_GameUISubsystem;
};
