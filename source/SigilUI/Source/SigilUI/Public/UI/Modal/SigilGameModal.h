// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "SigilGameModalTypes.h"
#include "SigilGameModal.generated.h"

class UCommonTextBlock;
class UDynamicEntryBox;
class USigilGameModalWidget;

/**
 * Definition for a modal dialog.
 * 模态对话框的定义。
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Const)
class SIGILUI_API USigilModalDefinition : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Header text for the modal.
	 * 模态对话框的标题文本。
	 */
	UPROPERTY(EditAnywhere, Category="GUIS", BlueprintReadWrite)
	FText Header;

	/**
	 * Body text for the modal.
	 * 模态对话框的正文文本。
	 */
	UPROPERTY(EditAnywhere, Category="GUIS", BlueprintReadWrite)
	FText Body;

	/**
	 * Widget class used to represent the modal.
	 * 表示模态对话框的小部件类。
	 */
	UPROPERTY(EditAnywhere, Category="GUIS", BlueprintReadWrite)
	TSoftClassPtr<USigilGameModalWidget> ModalWidget;

	/**
	 * Map of modal actions to their configurations.
	 * 模态动作及其配置的映射。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GUIS", meta = (ForceInlineRow, Categories = "Sigil.UI.Modal.Action"))
	TMap<FGameplayTag, FSigilGameModalAction> ModalActions;
};

/**
 * Base widget for modal dialogs.
 * 模态对话框的基础小部件。
 * @note Must bind a DynamicEntryBox named "EntryBox_Buttons" for button registration.
 * @注意 必须绑定一个名为"EntryBox_Buttons"的DynamicEntryBox以注册按钮。
 */
UCLASS(Abstract, meta = (Category = "Generic UI"))
class SIGILUI_API USigilGameModalWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * Constructor for the modal widget.
	 * 模态小部件构造函数。
	 */
	USigilGameModalWidget();

	/**
	 * Sets up the modal with the provided definition.
	 * 使用提供的定义设置模态对话框。
	 * @param ModalDefinition The modal definition. 模态定义。
	 * @param ModalActionCallback Callback for modal actions. 模态动作回调。
	 */
	virtual void SetupModal(const USigilModalDefinition* ModalDefinition, FSigilModalActionResultSignature ModalActionCallback);

	/**
	 * Closes the modal with the specified result.
	 * 以指定结果关闭模态对话框。
	 * @param ModalActionResult The modal action result. 模态动作结果。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS", meta = (Categories = "Sigil.UI.Modal.Action"))
	void CloseModal(FGameplayTag ModalActionResult);

	/**
	 * Terminates the modal.
	 * 终止模态对话框。
	 */
	virtual void KillModal();

protected:
	/**
	 * Event to apply modal definition data to UI elements.
	 * 将模态定义数据应用于UI元素的事件。
	 * @param ModalDefinition The modal definition. 模态定义。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="GUIS")
	void OnSetupModal(const USigilModalDefinition* ModalDefinition);

	/**
	 * Callback for modal action results.
	 * 模态动作结果的回调。
	 */
	FSigilModalActionResultSignature OnModalActionCallback;

private:
	/**
	 * Dynamic entry box for modal buttons.
	 * 模态按钮的动态入口框。
	 */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;

	/**
	 * Text block for the modal header.
	 * 模态标题的文本块。
	 */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Header;

	/**
	 * Text block for the modal body.
	 * 模态正文的文本块。
	 */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Body;
};