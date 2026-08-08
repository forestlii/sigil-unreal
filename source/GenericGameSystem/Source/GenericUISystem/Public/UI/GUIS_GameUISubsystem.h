// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "Input/UIActionBindingHandle.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GUIS_GameUIStructLibrary.h"
#include "UObject/SoftObjectPtr.h"
#include "GUIS_GameUISubsystem.generated.h"

class UCommonUserWidget;
class UGUIS_GameUIContext;
class FSubsystemCollectionBase;
class ULocalPlayer;
class UGUIS_GameUIPolicy;
class UObject;

/**
 * Game UI subsystem for managing UI policies and player UI interactions.
 * 管理UI策略和玩家UI交互的游戏UI子系统。
 * @details Intended to be subclassed for game-specific UI functionality.
 * @细节 旨在为特定游戏的UI功能进行子类化。
 */
UCLASS()
class GENERICUISYSTEM_API UGUIS_GameUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGUIS_GameUISubsystem()
	{
	}

	/**
	 * Initializes the UI subsystem.
	 * 初始化UI子系统。
	 * @param Collection The subsystem collection base. 子系统集合基类。
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Deinitializes the UI subsystem.
	 * 取消初始化UI子系统。
	 */
	virtual void Deinitialize() override;

	/**
	 * Determines if the subsystem should be created.
	 * 确定是否应创建子系统。
	 * @param Outer The outer object. 外部对象。
	 * @return True if the subsystem should be created, false otherwise. 如果应创建子系统则返回true，否则返回false。
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/**
	 * Gets the current UI policy (const version).
	 * 获取当前UI策略（常量版本）。
	 * @return The current UI policy. 当前UI策略。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS")
	const UGUIS_GameUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }

	/**
	 * Gets the current UI policy (non-const version).
	 * 获取当前UI策略（非常量版本）。
	 * @return The current UI policy. 当前UI策略。
	 */
	UGUIS_GameUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	/**
	 * Adds a player to the UI subsystem.
	 * 将玩家添加到UI子系统。
	 * @param LocalPlayer The local player to add. 要添加的本地玩家。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	virtual void AddPlayer(ULocalPlayer* LocalPlayer);

	/**
	 * Removes a player from the UI subsystem.
	 * 从UI子系统中移除玩家。
	 * @param LocalPlayer The local player to remove. 要移除的本地玩家。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	virtual void RemovePlayer(ULocalPlayer* LocalPlayer);

	/**
	 * Notifies the subsystem when a player is added.
	 * 当玩家被添加时通知子系统。
	 * @param LocalPlayer The added local player. 添加的本地玩家。
	 */
	virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);

	/**
	 * Notifies the subsystem when a player is removed.
	 * 当玩家被移除时通知子系统。
	 * @param LocalPlayer The removed local player. 移除的本地玩家。
	 */
	virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);

	/**
	 * Notifies the subsystem when a player is destroyed.
	 * 当玩家被销毁时通知子系统。
	 * @param LocalPlayer The destroyed local player. 销毁的本地玩家。
	 */
	virtual void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	/**
	 * Registers a UI action binding for a widget (deprecated).
	 * 为控件注册UI动作绑定（已弃用）。
	 * @param Target The target widget. 目标控件。
	 * @param InputAction The input action to bind. 要绑定的输入动作。
	 * @param bShouldDisplayInActionBar Whether to display in the action bar. 是否在动作栏中显示。
	 * @param Callback Delegate called when the action is executed. 动作执行时调用的委托。
	 * @param BindingHandle The handle for the binding. 绑定的句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DefaultToSelf="Target", DeprecatedFunction, DeprecationMessage="Use RegisterUIActionBindingForPlayer"))
	void RegisterUIActionBinding(UCommonUserWidget* Target, FDataTableRowHandle InputAction, bool bShouldDisplayInActionBar, const FGUIS_UIActionExecutedDelegate& Callback,
	                             FGUIS_UIActionBindingHandle& BindingHandle);

	/**
	 * Unregisters a UI action binding (deprecated).
	 * 取消注册UI动作绑定（已弃用）。
	 * @param BindingHandle The handle of the binding to unregister. 要取消注册的绑定句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DeprecatedFunction, DeprecationMessage="Use UnregisterUIActionBindingForPlayer"))
	void UnregisterBinding(UPARAM(ref) FGUIS_UIActionBindingHandle& BindingHandle);

	/**
	 * Registers a UI action binding for a specific player.
	 * 为特定玩家注册UI动作绑定。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Target The target widget. 目标控件。
	 * @param InputAction The input action to bind. 要绑定的输入动作。
	 * @param bShouldDisplayInActionBar Whether to display in the action bar. 是否在动作栏中显示。
	 * @param Callback Delegate called when the action is executed. 动作执行时调用的委托。
	 * @param BindingHandle The handle for the binding. 绑定的句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DefaultToSelf="Target"))
	virtual void RegisterUIActionBindingForPlayer(ULocalPlayer* LocalPlayer, UCommonUserWidget* Target, FDataTableRowHandle InputAction, bool bShouldDisplayInActionBar,
	                                              const FGUIS_UIActionExecutedDelegate& Callback,
	                                              FGUIS_UIActionBindingHandle& BindingHandle);

	/**
	 * Unregisters a UI action binding for a specific player.
	 * 为特定玩家取消注册UI动作绑定。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param BindingHandle The handle of the binding to unregister. 要取消注册的绑定句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	virtual void UnregisterUIActionBindingForPlayer(ULocalPlayer* LocalPlayer, UPARAM(ref) FGUIS_UIActionBindingHandle& BindingHandle);

	/**
	 * Registers a UI context for a specific player.
	 * 为特定玩家注册UI上下文。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param Context The UI context to register. 要注册的UI上下文。
	 * @param BindingHandle The handle for the context binding. 上下文绑定的句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DefaultToSelf="LocalPlayer"))
	void RegisterUIContextForPlayer(ULocalPlayer* LocalPlayer, UGUIS_GameUIContext* Context, FGUIS_UIContextBindingHandle& BindingHandle);

	/**
	 * Registers a UI context for an actor.
	 * 为演员注册UI上下文。
	 * @param Actor The actor to associate with the context. 与上下文关联的演员。
	 * @param Context The UI context to register. 要注册的UI上下文。
	 * @param BindingHandle The handle for the context binding. 上下文绑定的句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DefaultToSelf="Actor"))
	void RegisterUIContextForActor(AActor* Actor, UGUIS_GameUIContext* Context, FGUIS_UIContextBindingHandle& BindingHandle);

	/**
	 * Finds a UI context for a specific player.
	 * 为特定玩家查找UI上下文。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param ContextClass The class of the context to find. 要查找的上下文类。
	 * @param OutContext The found UI context (output). 找到的UI上下文（输出）。
	 * @return True if the context was found, false otherwise. 如果找到上下文则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DefaultToSelf="LocalPlayer", DeterminesOutputType="ContextClass", DynamicOutputParam="OutContext", ExpandBoolAsExecs="ReturnValue"))
	bool FindUIContextForPlayer(ULocalPlayer* LocalPlayer, TSubclassOf<UGUIS_GameUIContext> ContextClass, UGUIS_GameUIContext*& OutContext);

	/**
	 * Finds a UI context from a binding handle.
	 * 通过绑定句柄查找UI上下文。
	 * @param BindingHandle The binding handle to query. 要查询的绑定句柄。
	 * @param ContextClass The class of the context to find. 要查找的上下文类。
	 * @param OutContext The found UI context (output). 找到的UI上下文（输出）。
	 * @return True if the context was found, false otherwise. 如果找到上下文则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DefaultToSelf="LocalPlayer", DeterminesOutputType="ContextClass", DynamicOutputParam="OutContext", ExpandBoolAsExecs="ReturnValue"))
	bool FindUIContextFromHandle(UPARAM(ref) FGUIS_UIContextBindingHandle& BindingHandle, TSubclassOf<UGUIS_GameUIContext> ContextClass, UGUIS_GameUIContext*& OutContext);

	/**
	 * Unregisters a UI context for a specific player.
	 * 为特定玩家取消注册UI上下文。
	 * @param BindingHandle The handle of the context binding to unregister. 要取消注册的上下文绑定句柄。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta=(DefaultToSelf="LocalPlayer"))
	void UnregisterUIContextForPlayer(UPARAM(ref) FGUIS_UIContextBindingHandle& BindingHandle);

protected:
	/**
	 * Switches to a specified UI policy.
	 * 切换到指定的UI策略。
	 * @param InPolicy The UI policy to switch to. 要切换到的UI策略。
	 */
	void SwitchToPolicy(UGUIS_GameUIPolicy* InPolicy);

private:
	/**
	 * The current UI policy in use.
	 * 当前使用的UI策略。
	 */
	UPROPERTY(Transient)
	TObjectPtr<UGUIS_GameUIPolicy> CurrentPolicy = nullptr;

	/**
	 * Array of UI action binding handles.
	 * UI动作绑定句柄的数组。
	 */
	TArray<FUIActionBindingHandle> BindingHandles;
};