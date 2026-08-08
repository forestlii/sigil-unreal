// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "SigilInteractionDefinition.generated.h"

/**
 * Base class for interaction settings, used in smart object interaction entrances.
 * 交互设置基类，用于智能对象交互入口。
 */
UCLASS(BlueprintType, Blueprintable)
class SIGILINTERACTION_API USigilInteractionDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Display text for the interaction.
	 * 交互的显示文本。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText Text;

	/**
	 * Sub-text for the interaction.
	 * 交互的子文本。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText SubText;

	/**
	 * Input action that triggers the interaction.
	 * 触发交互的输入动作。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle TriggeringInputAction;
};
