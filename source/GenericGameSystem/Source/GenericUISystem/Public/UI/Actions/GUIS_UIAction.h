// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GUIS_UIAction.generated.h"

class UGUIS_ModalDefinition;
class UGUIS_UIAction;

/**
 * Base ui action for single data.
 */
UCLASS(Blueprintable, EditInlineNew, CollapseCategories, DefaultToInstanced, Abstract, Const)
class GENERICUISYSTEM_API UGUIS_UIAction : public UObject
{
	GENERATED_UCLASS_BODY()
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	bool IsCompatible(const UObject* Data) const;

	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	bool CanInvoke(const UObject* Data, APlayerController* PlayerController) const;

	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	void InvokeAction(const UObject* Data, APlayerController* PlayerController) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS|UIAction")
	FText GetActionName() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GUIS|UIAction")
	FName GetActionID() const;

	const FDataTableRowHandle& GetInputActionData() const { return InputActionData; }

	bool GetShouldDisplayInActionBar() const { return bShouldDisplayInActionBar; }

	bool GetRequiresConfirmation() const { return bRequiresConfirmation; }

	TSoftClassPtr<UGUIS_ModalDefinition> GetConfirmationModalClass() const { return ConfirmationModalClass; };

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "UIAction", meta = (DisplayName = "Is Compatible"))
	bool IsCompatibleInternal(const UObject* Data) const;

	UFUNCTION(BlueprintNativeEvent, Category = "UIAction", meta = (DisplayName = "Can Invoke"))
	bool CanInvokeInternal(const UObject* Data, APlayerController* PlayerController) const;

	UFUNCTION(BlueprintNativeEvent, Category = "UIAction", meta = (DisplayName = "Invoke Action"))
	void InvokeActionInternal(const UObject* Data, APlayerController* PlayerController) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIAction")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIAction")
	FName ActionID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIAction", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle InputActionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UIAction")
	bool bShouldDisplayInActionBar{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UIAction")
	bool bRequiresConfirmation{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UIAction", meta=(EditCondition="bRequiresConfirmation"))
	TSoftClassPtr<UGUIS_ModalDefinition> ConfirmationModalClass{nullptr};
};
