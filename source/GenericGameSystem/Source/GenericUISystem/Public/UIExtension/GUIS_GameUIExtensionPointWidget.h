// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "GUIS_GameUIExtensionSubsystem.h"
#include "Components/DynamicEntryBoxBase.h"
#include "GUIS_GameUIExtensionPointWidget.generated.h"

class IWidgetCompilerLog;

/**
 * Delegate for retrieving widget class for data.
 * 获取数据小部件类的委托。
 */
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(TSubclassOf<UUserWidget>, FOnGetWidgetClassForData, UObject*, DataItem);

/**
 * Delegate for configuring widget for data.
 * 配置数据小部件的委托。
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnConfigureWidgetForData, UUserWidget*, Widget, UObject*, DataItem);

/**
 * Widget representing a UI extension point in a layout.
 * 表示布局中UI扩展点的小部件。
 * @note Context is LocalPlayer.
 * @注意 上下文是LocalPlayer。
 */
UCLASS(meta=(Category = "Generic UI"))
class GENERICUISYSTEM_API UGUIS_GameUIExtensionPointWidget : public UDynamicEntryBoxBase
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the extension point widget.
	 * 扩展点小部件构造函数。
	 */
	UGUIS_GameUIExtensionPointWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Releases Slate resources.
	 * 释放Slate资源。
	 * @param bReleaseChildren Whether to release child resources. 是否释放子资源。
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/**
	 * Rebuilds the Slate widget.
	 * 重建Slate小部件。
	 * @return The rebuilt Slate widget. 重建的Slate小部件。
	 */
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/**
	 * Registers the widget for the player state if ready.
	 * 如果准备好，为玩家状态注册小部件。
	 */
	void RegisterForPlayerStateIfReady();

	/**
	 * Checks the player state.
	 * 检查玩家状态。
	 * @return True if player state is valid, false otherwise. 如果玩家状态有效返回true，否则返回false。
	 */
	bool CheckPlayerState();

	/**
	 * Called to check the player state.
	 * 检查玩家状态时调用。
	 */
	void OnCheckPlayerState();

	/**
	 * Timer handle for player state checks.
	 * 玩家状态检查的定时器句柄。
	 */
	FTimerHandle TimerHandle;

#if WITH_EDITOR
	/**
	 * Validates compiled defaults in the editor.
	 * 在编辑器中验证编译默认值。
	 * @param CompileLog The widget compiler log. 小部件编译日志。
	 */
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

private:
	/**
	 * Resets the extension point.
	 * 重置扩展点。
	 */
	void ResetExtensionPoint();

	/**
	 * Registers the extension point.
	 * 注册扩展点。
	 */
	void RegisterExtensionPoint();

	/**
	 * Registers the extension point for a specific player state.
	 * 为特定玩家状态注册扩展点。
	 * @param LocalPlayer The local player. 本地玩家。
	 * @param PlayerState The player state. 玩家状态。
	 */
	void RegisterExtensionPointForPlayerState(ULocalPlayer* LocalPlayer, APlayerState* PlayerState);

	/**
	 * Loads allowed data classes.
	 * 加载允许的数据类。
	 * @return The allowed data classes. 允许的数据类。
	 */
	TArray<UClass*> LoadAllowedDataClasses() const;

	/**
	 * Called when an extension is added or removed.
	 * 扩展添加或移除时调用。
	 * @param Action The extension action (Added/Removed). 扩展动作（添加/移除）。
	 * @param Request The extension request. 扩展请求。
	 */
	void OnAddOrRemoveExtension(EGUIS_GameUIExtAction Action, const FGUIS_GameUIExtRequest& Request);

protected:
	/**
	 * Tag defining the extension point.
	 * 定义扩展点的标签。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	FGameplayTag ExtensionPointTag;

	/**
	 * Match type for the extension point tag.
	 * 扩展点标签的匹配类型。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	EGUIS_GameUIExtPointMatchType ExtensionPointTagMatch = EGUIS_GameUIExtPointMatchType::ExactMatch;

	/**
	 * Allowed data classes for the extension point.
	 * 扩展点允许的数据类。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<TSoftClassPtr<UObject>> DataClasses;

	/**
	 * Event to get the widget class for non-widget data.
	 * 为非小部件数据获取小部件类的事件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnGetWidgetClassForData GetWidgetClassForData;

	/**
	 * Event to configure widget instance for non-widget data.
	 * 为非小部件数据配置小部件实例的事件。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnConfigureWidgetForData ConfigureWidgetForData;

	/**
	 * Array of extension point handles.
	 * 扩展点句柄数组。
	 */
	TArray<FGUIS_GameUIExtPointHandle> ExtensionPointHandles;

	/**
	 * Mapping of extension handles to widgets.
	 * 扩展句柄到小部件的映射。
	 */
	UPROPERTY(Transient)
	TMap<FGUIS_GameUIExtHandle, TObjectPtr<UUserWidget>> ExtensionMapping;
};