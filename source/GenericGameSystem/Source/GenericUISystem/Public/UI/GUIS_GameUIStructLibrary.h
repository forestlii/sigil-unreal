// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/UIActionBindingHandle.h"
#include "UObject/Object.h"
#include "GUIS_GameUIStructLibrary.generated.h"

class UGUIS_GameUIContext;
class UGUIS_GameUILayout;
class ULocalPlayer;

/**
 * Delegate for handling UI action execution.
 * 处理UI动作执行的委托。
 * @param ActionName The name of the executed action. 执行的动作名称。
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FGUIS_UIActionExecutedDelegate, FName, ActionName);

/**
 * Struct for storing UI action binding information.
 * 存储UI动作绑定信息的结构体。
 */
USTRUCT(BlueprintType)
struct FGUIS_UIActionBindingHandle
{
	GENERATED_BODY()

	/**
	 * Unique identifier for the binding.
	 * 绑定的唯一标识符。
	 */
	FName Id;

	/**
	 * Handle for the UI action binding.
	 * UI动作绑定的句柄。
	 */
	FUIActionBindingHandle Handle;
};

/**
 * Struct for storing UI context binding information.
 * 存储UI上下文绑定信息的结构体。
 */
USTRUCT(BlueprintType)
struct FGUIS_UIContextBindingHandle
{
	GENERATED_BODY()

	FGUIS_UIContextBindingHandle()
	{
	};

	/**
	 * Constructor for UI context binding handle.
	 * UI上下文绑定句柄的构造函数。
	 * @param InLocalPlayer The local player associated with the context. 与上下文关联的本地玩家。
	 * @param InContextClass The class of the context. 上下文的类。
	 */
	FGUIS_UIContextBindingHandle(ULocalPlayer* InLocalPlayer, UClass* InContextClass);

	/**
	 * The local player associated with the context.
	 * 与上下文关联的本地玩家。
	 */
	UPROPERTY()
	TObjectPtr<ULocalPlayer> LocalPlayer;

	/**
	 * The class of the UI context.
	 * UI上下文的类。
	 */
	UPROPERTY()
	UClass* ContextClass{nullptr};
};

/**
 * Struct for storing root viewport layout information.
 * 存储根视口布局信息的结构体。
 */
USTRUCT()
struct FGUIS_RootViewportLayoutInfo
{
	GENERATED_BODY()

	/**
	 * The local player associated with the layout.
	 * 与布局关联的本地玩家。
	 */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	/**
	 * The root layout widget.
	 * 根布局控件。
	 */
	UPROPERTY(Transient)
	TObjectPtr<UGUIS_GameUILayout> RootLayout = nullptr;

	/**
	 * Indicates if the layout is added to the viewport.
	 * 指示布局是否已添加到视口。
	 */
	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	/**
	 * Array of UI contexts associated with the layout.
	 * 与布局关联的UI上下文数组。
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGUIS_GameUIContext>> Contexts;

	/**
	 * Array of UI action binding handles.
	 * UI动作绑定句柄的数组。
	 */
	UPROPERTY(Transient)
	TArray<FUIActionBindingHandle> BindingHandles;

	FGUIS_RootViewportLayoutInfo()
	{
	}

	/**
	 * Constructor for root viewport layout information.
	 * 根视口布局信息的构造函数。
	 * @param InLocalPlayer The local player. 本地玩家。
	 * @param InRootLayout The root layout widget. 根布局控件。
	 * @param bIsInViewport Whether the layout is in the viewport. 布局是否在视口中。
	 */
	FGUIS_RootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UGUIS_GameUILayout* InRootLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer), RootLayout(InRootLayout), bAddedToViewport(bIsInViewport)
	{
	}

	/**
	 * Equality operator to compare with a local player.
	 * 与本地玩家比较的相等运算符。
	 * @param OtherLocalPlayer The local player to compare with. 要比较的本地玩家。
	 * @return True if the local players match, false otherwise. 如果本地玩家匹配则返回true，否则返回false。
	 */
	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};
