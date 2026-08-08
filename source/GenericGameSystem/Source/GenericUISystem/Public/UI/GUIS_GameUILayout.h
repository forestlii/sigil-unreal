// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameplayTagContainer.h"
#include "GUIS_GameUIFunctionLibrary.h"
#include "Widgets/CommonActivatableWidgetContainer.h" // IWYU pragma: keep

#include "GUIS_GameUILayout.generated.h"

class APlayerController;
class UClass;
class UCommonActivatableWidgetContainerBase;
class ULocalPlayer;
class UObject;
struct FFrame;

/**
 * Enumeration for the state of an async UI widget layer load operation.
 * 异步UI控件层加载操作状态的枚举。
 */
enum class EGUIS_AsyncWidgetLayerState : uint8
{
	Canceled,   // Operation was canceled. 操作被取消。
	Initialize, // Widget is being initialized. 控件正在初始化。
	AfterPush   // Widget has been pushed to the layer. 控件已被推送到层。
};

/**
 * Primary game UI layout for managing widget layers for a single player.
 * 管理单个玩家控件层的游戏主UI布局。
 * @details Handles the layout, pushing, and display of UI layers for a player in a split-screen game.
 * @细节 处理分屏游戏中玩家的UI层布局、推送和显示。
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class GENERICUISYSTEM_API UGUIS_GameUILayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the game UI layout.
	 * 游戏UI布局的构造函数。
	 */
	UGUIS_GameUILayout(const FObjectInitializer &ObjectInitializer);

	/**
	 * Sets the dormant state of the layout, limiting it to persistent actions.
	 * 设置布局的休眠状态，限制为持久动作。
	 * @param Dormant True to set the layout as dormant, false otherwise. 如果设置为休眠则为true，否则为false。
	 */
	void SetIsDormant(bool Dormant);

	/**
	 * Checks if the layout is in a dormant state.
	 * 检查布局是否处于休眠状态。
	 * @return True if the layout is dormant, false otherwise. 如果布局休眠则返回true，否则返回false。
	 */
	bool IsDormant() const { return bIsDormant; }

public:
	/**
	 * Asynchronously pushes a widget to a specified layer.
	 * 异步将控件推送到指定层。
	 * @param LayerName The tag of the layer to push to. 要推送到的层标签。
	 * @param bSuspendInputUntilComplete Whether to suspend input until loading is complete. 是否在加载完成前暂停输入。
	 * @param ActivatableWidgetClass The class of the widget to push. 要推送的控件类。
	 * @return A handle to the async streaming operation. 异步流操作的句柄。
	 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EGUIS_AsyncWidgetLayerState, ActivatableWidgetT *) {});
	}

	/**
	 * Asynchronously pushes a widget to a specified layer with a state callback.
	 * 使用状态回调异步将控件推送到指定层。
	 * @param LayerName The tag of the layer to push to. 要推送到的层标签。
	 * @param bSuspendInputUntilComplete Whether to suspend input until loading is complete. 是否在加载完成前暂停输入。
	 * @param ActivatableWidgetClass The class of the widget to push. 要推送的控件类。
	 * @param StateFunc Callback for handling widget state changes. 处理控件状态变化的回调。
	 * @return A handle to the async streaming operation. 异步流操作的句柄。
	 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
															  TFunction<void(EGUIS_AsyncWidgetLayerState, ActivatableWidgetT *)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UGUIS_GameUIFunctionLibrary::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		FStreamableManager &StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
																																											[this, LayerName, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
																																											{
																																												UGUIS_GameUIFunctionLibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

																																												ActivatableWidgetT *Widget = PushWidgetToLayerStack<ActivatableWidgetT>(
																																													LayerName, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT &WidgetToInit)
																																													{ StateFunc(EGUIS_AsyncWidgetLayerState::Initialize, &WidgetToInit); });

																																												StateFunc(EGUIS_AsyncWidgetLayerState::AfterPush, Widget);
																																											}));

		// Setup a cancel delegate to resume input if the operation is canceled.
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
																				  [this, StateFunc, SuspendInputToken]()
																				  {
																					  UGUIS_GameUIFunctionLibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
																					  StateFunc(EGUIS_AsyncWidgetLayerState::Canceled, nullptr);
																				  }));

		return StreamingHandle;
	}

	/**
	 * Pushes a widget to a specified layer.
	 * 将控件推送到指定层。
	 * @param LayerName The tag of the layer to push to. 要推送到的层标签。
	 * @param ActivatableWidgetClass The class of the widget to push. 要推送的控件类。
	 * @return Pointer to the pushed widget, or nullptr if failed. 推送的控件指针，失败时为nullptr。
	 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT *PushWidgetToLayerStack(FGameplayTag LayerName, UClass *ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT &) {});
	}

	/**
	 * Pushes a widget to a specified layer with initialization.
	 * 将控件推送到指定层并进行初始化。
	 * @param LayerName The tag of the layer to push to. 要推送到的层标签。
	 * @param ActivatableWidgetClass The class of the widget to push. 要推送的控件类。
	 * @param InitInstanceFunc Function to initialize the widget instance. 初始化控件实例的函数。
	 * @return Pointer to the pushed widget, or nullptr if failed. 推送的控件指针，失败时为nullptr。
	 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT *PushWidgetToLayerStack(FGameplayTag LayerName, UClass *ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT &)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase *Layer = GetLayerWidget(LayerName))
		{
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
		}

		return nullptr;
	}

	/**
	 * Finds and removes a widget from any layer.
	 * 从任意层查找并移除控件。
	 * @param ActivatableWidget The widget to remove. 要移除的控件。
	 */
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget *ActivatableWidget);

	/**
	 * Retrieves the layer widget for a given layer tag.
	 * 获取给定层标签的层控件。
	 * @param LayerName The tag of the layer to retrieve. 要检索的层标签。
	 * @return Pointer to the layer widget, or nullptr if not found. 层控件指针，未找到时为nullptr。
	 */
	UCommonActivatableWidgetContainerBase *GetLayerWidget(FGameplayTag LayerName);

protected:
	/**
	 * Registers a layer for pushing widgets.
	 * 注册一个用于推送控件的层。
	 * @param LayerTag The tag identifying the layer. 标识层的标签。
	 * @param LayerWidget The widget container for the layer. 层的控件容器。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer,GUIS.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase *LayerWidget);

	/**
	 * Called when the dormant state changes.
	 * 当休眠状态更改时调用。
	 */
	virtual void OnIsDormantChanged();

	/**
	 * Handles widget stack transitioning events.
	 * 处理控件堆栈转换事件。
	 * @param Widget The widget container transitioning. 转换的控件容器。
	 * @param bIsTransitioning True if transitioning, false otherwise. 如果正在转换则为true，否则为false。
	 */
	void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase *Widget, bool bIsTransitioning);

private:
	/**
	 * Indicates if the layout is dormant.
	 * 指示布局是否休眠。
	 */
	bool bIsDormant = false;

	/**
	 * Tracks all suspended input tokens for async UI loading.
	 * 跟踪异步UI加载的所有暂停输入令牌。
	 */
	TArray<FName> SuspendInputTokens;

	/**
	 * Registered layers for the primary layout.
	 * 主布局的注册层。
	 */
	UPROPERTY(Transient, meta = (Categories = "UI.Layer,GUIS.Layer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
};