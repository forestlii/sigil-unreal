// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonListView.h"
#include "SigilListView.generated.h"

class USigilWidgetFactory;

/**
 * Extended ListView allowing dynamic selection of entry widget class via data asset.
 * 通过数据资产动态选择入口小部件类的扩展ListView。
 */
UCLASS(Blueprintable, meta = (Category = "Generic UI"))
class SIGILUI_API USigilListView : public UCommonListView
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the ListView widget.
	 * ListView小部件构造函数。
	 */
	USigilListView(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	/**
	 * Validates compiled defaults in the editor.
	 * 在编辑器中验证编译默认值。
	 * @param InCompileLog The widget compiler log. 小部件编译日志。
	 */
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const override;
#endif

	/**
	 * Sets the entry widget factories for dynamic widget selection.
	 * 设置动态小部件选择的入口小部件工厂。
	 * @param NewFactories The array of widget factories. 小部件工厂数组。
	 */
	UFUNCTION(BlueprintCallable, Category = "ListEntries")
	void SetEntryWidgetFactories(TArray<USigilWidgetFactory*> NewFactories);

protected:
	/**
	 * Generates an entry widget for the specified item.
	 * 为指定项生成入口小部件。
	 * @param Item The item to generate a widget for. 要生成小部件的项。
	 * @param DesiredEntryClass The desired entry widget class. 期望的入口小部件类。
	 * @param OwnerTable The owning table view. 所属表格视图。
	 * @return The generated widget. 生成的小部件。
	 */
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;

	/**
	 * Array of widget factories for dynamic entry widget selection.
	 * 动态入口小部件选择的小部件工厂数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ListEntries", meta = (IsBindableEvent = "True"))
	TArray<TObjectPtr<USigilWidgetFactory>> EntryWidgetFactories;
};