// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "SigilTabDefinition.generated.h"

class UCommonActivatableWidget;
class UWidget;
class UCommonUserWidget;
class UCommonButtonBase;

/**
 * Base Tab definition.
 * @attention Deprecated as it's unstable.
 * 基础选项卡定义。
 * @注意 已经弃用
 */
UCLASS(Blueprintable, EditInlineNew, CollapseCategories, Const, DefaultToInstanced, Deprecated)
class SIGILUI_API UDEPRECATED_SigilTabDefinition : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	FName TabId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	FText TabText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	FSlateBrush IconBrush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition", Transient)
	bool bHidden;

	/**
	 * A common button which implements SigilTabButtonInterface to received Label infomation.
	 * 指定用作Tab按钮的Widget类型，该类型必须实现SigilTabButtonInterface以接收Label信息。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition", meta = (MustImplement = "/Script/SigilUI.SigilTabButtonInterface", AllowAbstract = "false"))
	TSoftClassPtr<UCommonButtonBase> TabButtonType;

	/**
	 * 该所呈现的Widget（可选），如果有指定，那么需要调用
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tab Definition")
	TSoftClassPtr<UCommonActivatableWidget> TabContentType;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> CreatedTabContentWidget;
};
