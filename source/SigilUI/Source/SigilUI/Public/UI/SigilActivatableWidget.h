// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "SigilActivatableWidget.generated.h"

struct FUIInputConfig;

/**
 * Enum defining input modes for activatable widgets.
 * 定义可激活小部件输入模式的枚举。
 */
UENUM()
enum class ESigilActivatableWidgetInputMode : uint8
{
	/**
	 * Default input mode.
	 * 默认输入模式。
	 */
	Default,

	/**
	 * Allows both game and menu input.
	 * 允许游戏和菜单输入。
	 */
	GameAndMenu,

	/**
	 * Game input only.
	 * 仅游戏输入。
	 */
	Game,

	/**
	 * Menu input only.
	 * 仅菜单输入。
	 */
	Menu
};

/**
 * Activatable widget that manages input configuration when activated.
 * 可激活小部件，激活时管理输入配置。
 */
UCLASS(Abstract, Blueprintable)
class SIGILUI_API USigilActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the activatable widget.
	 * 可激活小部件构造函数。
	 */
	USigilActivatableWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Sets whether the widget handles back navigation.
	 * 设置小部件是否处理后退导航。
	 * @param bNewState The new back handler state. 新的后退处理状态。
	 */
	UFUNCTION(BlueprintCallable, Category = "GUIS|ActivatableWidget")
	void SetIsBackHandler(bool bNewState);

	/**
	 * Retrieves the desired input configuration.
	 * 获取期望的输入配置。
	 * @return The input configuration. 输入配置。
	 */
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

#if WITH_EDITOR
	/**
	 * Validates the compiled widget tree in the editor.
	 * 在编辑器中验证编译的小部件树。
	 * @param BlueprintWidgetTree The widget tree to validate. 要验证的小部件树。
	 * @param CompileLog The widget compiler log. 小部件编译日志。
	 */
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

protected:
	/**
	 * Desired input mode when the widget is activated.
	 * 小部件激活时的期望输入模式。
	 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	ESigilActivatableWidgetInputMode InputConfig = ESigilActivatableWidgetInputMode::Default;

	/**
	 * Mouse capture behavior for game input.
	 * 游戏输入的鼠标捕获行为。
	 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};