// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CommonTabListWidgetBase.h"
#include "GUIS_TabListWidgetBase.generated.h"

class UDEPRECATED_GUIS_TabDefinition;
class UCommonButtonBase;
class UCommonUserWidget;
class UObject;
class UWidget;
struct FFrame;

/**
 * Structure defining a tab descriptor.
 * 定义选项卡描述的结构。
 */
USTRUCT(BlueprintType)
struct GENERICUISYSTEM_API FGUIS_TabDescriptor
{
	GENERATED_BODY()

	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	FGUIS_TabDescriptor();

	/**
	 * Unique identifier for the tab.
	 * 选项卡的唯一标识符。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	FName TabId;

	/**
	 * Display text for the tab.
	 * 选项卡的显示文本。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	FText TabText;

	/**
	 * Icon brush for the tab.
	 * 选项卡的图标画刷。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	FSlateBrush IconBrush;

	/**
	 * Whether the tab is hidden.
	 * 选项卡是否隐藏。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition", Transient)
	bool bHidden{false};

	/**
	 * Button type for the tab, must implement GUIS_TabButtonInterface.
	 * 选项卡的按钮类型，必须实现GUIS_TabButtonInterface。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition", meta = (MustImplement = "/Script/GenericUISystem.GUIS_TabButtonInterface", AllowAbstract = "false"))
	TSoftClassPtr<UCommonButtonBase> TabButtonType;

	/**
	 * Content widget type for the tab (optional).
	 * 选项卡的内容小部件类型（可选）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	TSoftClassPtr<UCommonUserWidget> TabContentType;

	/**
	 * The created content widget for the tab.
	 * 选项卡的已创建内容小部件。
	 */
	UPROPERTY(Transient)
	TObjectPtr<UWidget> CreatedTabContentWidget;
};

/**
 * Interface for tab buttons.
 * 选项卡按钮接口。
 */
UINTERFACE(BlueprintType)
class GENERICUISYSTEM_API UGUIS_TabButtonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for tab buttons to receive and update tab label information.
 * 选项卡按钮接收和更新标签信息的接口。
 */
class GENERICUISYSTEM_API IGUIS_TabButtonInterface
{
	GENERATED_BODY()

public:
	/**
	 * Sets the deprecated tab definition (no longer used).
	 * 设置已弃用的选项卡定义（不再使用）。
	 * @param TabDefinition The tab definition. 选项卡定义。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Button", meta=(DeprecatedFunction, DeprecationMessage="Use Set TabLabelInfo"))
	void SetTabDefinition(const UDEPRECATED_GUIS_TabDefinition* TabDefinition);

	/**
	 * Sets the tab label information.
	 * 设置选项卡标签信息。
	 * @param TabDescriptor The tab descriptor. 选项卡描述。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Button")
	void SetTabLabelInfo(const FGUIS_TabDescriptor& TabDescriptor);
};

/**
 * Tab list widget for managing and displaying tabs.
 * 管理和显示选项卡的选项卡列表小部件。
 * @note Can be linked to a switcher for dynamic tab switching.
 * @注意 可关联到切换器以实现动态选项卡切换。
 */
UCLASS(Blueprintable, BlueprintType, Abstract, meta = (DisableNativeTick))
class GENERICUISYSTEM_API UGUIS_TabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the tab list widget.
	 * 选项卡列表小部件构造函数。
	 */
	UGUIS_TabListWidgetBase();

	/**
	 * Retrieves preregistered tab information by ID.
	 * 通过ID获取预注册的选项卡信息。
	 * @param TabNameId The tab ID. 选项卡ID。
	 * @param OutTabInfo The tab descriptor (output). 选项卡描述（输出）。
	 * @return True if found, false otherwise. 如果找到返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS|TabList")
	bool GetPreregisteredTabInfo(const FName TabNameId, FGUIS_TabDescriptor& OutTabInfo);

	/**
	 * Retrieves the index of a preregistered tab by ID.
	 * 通过ID获取预注册选项卡的索引。
	 * @param TabNameId The tab ID. 选项卡ID。
	 * @return The tab index. 选项卡索引。
	 */
	UFUNCTION(BlueprintCallable, Category = "GUIS|TabList")
	int32 GetPreregisteredTabIndex(FName TabNameId) const;

	/**
	 * Finds preregistered tab information by ID.
	 * 通过ID查找预注册的选项卡信息。
	 * @param TabNameId The tab ID. 选项卡ID。
	 * @param OutTabInfo The tab descriptor (output). 选项卡描述（输出）。
	 * @return True if found, false otherwise. 如果找到返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "GUIS|TabList", meta=(ExpandBoolAsExecs="ReturnValue"))
	bool FindPreregisteredTabInfo(const FName TabNameId, FGUIS_TabDescriptor& OutTabInfo);

	/**
	 * Toggles the hidden state of a tab (call before associating a switcher).
	 * 切换选项卡的隐藏状态（在关联切换器之前调用）。
	 * @param TabNameId The tab ID. 选项卡ID。
	 * @param bHidden Whether to hide the tab. 是否隐藏选项卡。
	 */
	UFUNCTION(BlueprintCallable, Category = "GUIS|TabList")
	void SetTabHiddenState(FName TabNameId, bool bHidden);

	/**
	 * Registers a dynamic tab.
	 * 注册动态选项卡。
	 * @param TabDescriptor The tab descriptor. 选项卡描述。
	 * @return True if successful, false otherwise. 如果成功返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS|TabList")
	bool RegisterDynamicTab(const FGUIS_TabDescriptor& TabDescriptor);

	/**
	 * Checks if the first tab is active.
	 * 检查第一个选项卡是否激活。
	 * @return True if the first tab is active. 如果第一个选项卡激活返回true。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS|TabList")
	bool IsFirstTabActive() const;

	/**
	 * Checks if the last tab is active.
	 * 检查最后一个选项卡是否激活。
	 * @return True if the last tab is active. 如果最后一个选项卡激活返回true。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS|TabList")
	bool IsLastTabActive() const;

	/**
	 * Checks if a tab is visible.
	 * 检查选项卡是否可见。
	 * @param TabId The tab ID. 选项卡ID。
	 * @return True if the tab is visible. 如果选项卡可见返回true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS|TabList")
	bool IsTabVisible(FName TabId);

	/**
	 * Retrieves the number of visible tabs.
	 * 获取可见选项卡的数量。
	 * @return The number of visible tabs. 可见选项卡数量。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS|TabList")
	int32 GetVisibleTabCount();

	/**
	 * Delegate for when a new tab is created.
	 * 新选项卡创建时的委托。
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabContentCreated, FName, TabId, UCommonUserWidget *, TabWidget);

	/**
	 * Native delegate for tab creation events.
	 * 选项卡创建事件的本地委托。
	 */
	DECLARE_EVENT_TwoParams(UGUIS_TabListWidgetBase, FOnTabContentCreatedNative, FName /* TabId */, UCommonUserWidget * /* TabWidget */);

	/**
	 * Broadcasts when a new tab is created.
	 * 新选项卡创建时广播。
	 */
	UPROPERTY(BlueprintAssignable, Category="GUIS|TabList")
	FOnTabContentCreated OnTabContentCreated;

	/**
	 * Native event for tab creation.
	 * 选项卡创建的本地事件。
	 */
	FOnTabContentCreatedNative OnTabContentCreatedNative;

protected:
	/**
	 * Called when the widget is initialized.
	 * 小部件初始化时调用。
	 */
	virtual void NativeOnInitialized() override;

	/**
	 * Called when the widget is constructed.
	 * 小部件构造时调用。
	 */
	virtual void NativeConstruct() override;

	/**
	 * Called when the widget is destructed.
	 * 小部件销毁时调用。
	 */
	virtual void NativeDestruct() override;

	/**
	 * Called before the linked switcher changes.
	 * 关联切换器更改前调用。
	 */
	virtual void HandlePreLinkedSwitcherChanged() override;

	/**
	 * Called after the linked switcher changes.
	 * 关联切换器更改后调用。
	 */
	virtual void HandlePostLinkedSwitcherChanged() override;

	/**
	 * Handles tab creation.
	 * 处理选项卡创建。
	 * @param TabId The tab ID. 选项卡ID。
	 * @param TabButton The tab button. 选项卡按钮。
	 */
	virtual void HandleTabCreation_Implementation(FName TabId, UCommonButtonBase* TabButton) override;

	/**
	 * Sets up the tabs.
	 * 设置选项卡。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS|TabList")
	void SetupTabs();

	/**
	 * Deprecated array of tab definitions.
	 * 已弃用的选项卡定义数组。
	 */
	UPROPERTY(Instanced, meta = (BlueprintProtected, TitleProperty = "TabId"))
	TArray<TObjectPtr<UDEPRECATED_GUIS_TabDefinition>> TabDefinitions_DEPRECATED;

	/**
	 * Array of preregistered tab descriptors.
	 * 预注册选项卡描述数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TabList", meta=(TitleProperty="TabId"))
	TArray<FGUIS_TabDescriptor> PreregisteredTabInfoArray;

	/**
	 * Map of pending tab label information for runtime-registered tabs.
	 * 运行时注册选项卡的待处理标签信息映射。
	 */
	UPROPERTY()
	TMap<FName, FGUIS_TabDescriptor> PendingTabLabelInfoMap;

#if WITH_EDITOR
	/**
	 * Called after loading in the editor.
	 * 编辑器中加载后调用。
	 */
	virtual void PostLoad() override;

	/**
	 * Validates compiled defaults in the editor.
	 * 在编辑器中验证编译默认值。
	 * @param CompileLog The widget compiler log. 小部件编译日志。
	 */
	virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const override;
#endif
};
