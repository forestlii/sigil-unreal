// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/DataTable.h"
#include "GUIS_GameModalTypes.generated.h"

class UGUIS_ButtonBase;
class FSubsystemCollectionBase;
class UGUIS_ModalDefinition;
class UObject;

/**
 * Delegate for modal action results.
 * 模态动作结果的委托。
 */
DECLARE_DELEGATE_OneParam(FGUIS_ModalActionResultSignature, FGameplayTag /* Result */);

/**
 * Configuration for a modal action.
 * 模态动作的配置。
 */
USTRUCT(BlueprintType)
struct GENERICUISYSTEM_API FGUIS_GameModalAction
{
	GENERATED_BODY()

	/**
		 * Display text for the modal action.
		 * 模态动作的显示文本。
		 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GUIS")
	FText DisplayText;

	/**
	 * Button widget class for the modal action.
	 * 模态动作的按钮小部件类。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GUIS", NoClear)
	TSoftClassPtr<UGUIS_ButtonBase> ButtonType;

	/**
	 * Input action associated with the modal action.
	 * 模态动作关联的输入动作。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GUIS", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle InputAction;
};
