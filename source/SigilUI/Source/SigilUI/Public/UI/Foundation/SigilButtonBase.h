// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "SigilButtonBase.generated.h"

/**
 * Base Button
 * 基础按钮
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(Category = "Generic UI"))
class SIGILUI_API USigilButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/**
	 * @param InText The override text to display on the button. 该Text会覆盖按钮上的文字。
	 */
	UFUNCTION(BlueprintCallable, Category="GUIS")
	void SetButtonText(const FText& InText);

protected:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	// UCommonButtonBase interface
	virtual void UpdateInputActionWidget() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	// End of UCommonButtonBase interface

	void RefreshButtonText();

	// 在PreConstruct,InputActionWidget更新，以及ButtonText变化时会触发。
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateButtonText(const FText& InText);

	/**
	 * Will use the text from InputActionWidget if not checked.
	 * 在PreConstruct,InputActionWidget更新，以及ButtonText变化时会触发。
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateButtonStyle();

	/**
	 * Will use the text from InputActionWidget if not checked.
	 * 不勾选的情况下，会使用来自InputActionWidget的显示文字。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button", meta = (ExposeOnSpawn = true, InlineEditConditionToggle))
	bool bOverride_ButtonText{true};

	/**
	 * The text to display on the button.
	 * 按钮的显示文字。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button", meta = (ExposeOnSpawn = true, EditCondition = "bOverride_ButtonText"))
	FText ButtonText;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif // WITH_EDITOR
};
