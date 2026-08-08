// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "Blueprint/UserWidgetPool.h"
#include "GUIS_ListEntryDetailView.generated.h"

class UGUIS_DetailSectionsBuilder;
class UGUIS_ListEntryDetailSection;
class UVerticalBox;
class UGUIS_WidgetFactory;
struct FStreamableHandle;

/**
 * Detail view containing multiple sections to represent an item (any UObject).
 * 包含多个细节部分的视图，用于展示一个项（任意UObject类型）。
 */
UCLASS(Abstract, meta=(Category = "Generic UI"))
class GENERICUISYSTEM_API UGUIS_ListEntryDetailView : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the detail view widget.
	 * 细节视图小部件构造函数。
	 */
	UGUIS_ListEntryDetailView(const FObjectInitializer& ObjectInitializer);

	/**
	 * Sets the object represented by this detail view as data.
	 * 设置此细节视图表示的对象作为数据。
	 * @param InListItemObject The object to display. 要显示的对象。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	void SetListItemObject(UObject* InListItemObject);

	/**
	 * Sets the associated detail sections builder.
	 * 设置关联的细节部分构建器。
	 * @param NewBuilder The detail sections builder. 细节部分构建器。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	void SetSectionsBuilder(UGUIS_DetailSectionsBuilder* NewBuilder);

	/**
	 * Releases Slate resources.
	 * 释放Slate资源。
	 * @param bReleaseChildren Whether to release child resources. 是否释放子资源。
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	/**
	 * Called when the widget is constructed.
	 * 小部件构造时调用。
	 */
	virtual void NativeConstruct() override;

	/**
	 * Called when the widget is initialized.
	 * 小部件初始化时调用。
	 */
	virtual void NativeOnInitialized() override;

	/**
	 * Creates a detail section extension for the specified data.
	 * 为指定数据创建细节部分扩展。
	 * @param InData The data object. 数据对象。
	 * @param SectionClass The section widget class. 部分小部件类。
	 */
	void CreateDetailsExtension(UObject* InData, TSubclassOf<UGUIS_ListEntryDetailSection> SectionClass);

	/**
	 * Detail sections builder for displaying data based on widget specifications.
	 * 根据小部件规格显示数据的细节部分构建器。
	 */
	UPROPERTY(EditAnywhere, Category="GUIS", meta = (AllowAbstract = false))
	TObjectPtr<UGUIS_DetailSectionsBuilder> SectionsBuilder;

	/**
	 * Pool for managing extension widgets.
	 * 管理扩展小部件的池。
	 */
	UPROPERTY(Transient)
	FUserWidgetPool ExtensionWidgetPool;

	/**
	 * Current object represented by the detail view.
	 * 细节视图当前表示的对象。
	 */
	UPROPERTY(Transient)
	TObjectPtr<UObject> CurrentListItemObject;

	/**
	 * Handle for streaming assets.
	 * 流式加载资产的句柄。
	 */
	TSharedPtr<FStreamableHandle> StreamingHandle;

private:
	/**
	 * Vertical box for detail sections.
	 * 细节部分的垂直框。
	 */
	UPROPERTY(BlueprintReadOnly, Category="GUIS", meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UVerticalBox> Box_DetailSections;
};