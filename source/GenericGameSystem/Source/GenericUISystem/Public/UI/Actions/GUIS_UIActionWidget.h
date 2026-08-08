// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "GUIS_UIAction.h"
#include "GUIS_UIActionWidget.generated.h"

class UGUIS_AsyncAction_ShowModel;
class UGUIS_UIActionFactory;


/**
 * A widget which can associate data with register ui action dynamically based on passed-in data.
 * @attention There's no visual for this widget.
 * 此Widget可以关联一个数据，并为其自动注册可用的输入事件。
 */
UCLASS(ClassGroup = GUIS, meta=(Category="Generic UI System"), AutoExpandCategories=("GUIS"))
class GENERICUISYSTEM_API UGUIS_UIActionWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	void SetAssociatedData(UObject* Data);

	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	void RegisterActions();

	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	virtual void RegisterActionsWithFactory(TSoftObjectPtr<UGUIS_UIActionFactory> InActionFactory);

	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	virtual void UnregisterActions();

	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	virtual void CancelAction();


	//~ Begin UWidget
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	//~ End UWidget interface

protected:
	UFUNCTION()
	virtual void HandleUIAction(const UGUIS_UIAction* Action);

	UFUNCTION()
	virtual void HandleModalAction(FGameplayTag ActionTag);

	UPROPERTY()
	TWeakObjectPtr<UObject> AssociatedData;

	/**
	 * A factory to get available ui actions for associated data.
	 * 该数据资产用于针对关联数据返回所有可用的ui操作。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UIAction")
	TObjectPtr<UGUIS_UIActionFactory> ActionFactory;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGUIS_HandleUIActionSignature, const UGUIS_UIAction*, ActionReference);

	UPROPERTY(BlueprintAssignable, Category="UIAction")
	FGUIS_HandleUIActionSignature OnHandleUIAction;

private:
	TArray<FUIActionBindingHandle> ActionBindings;

	UPROPERTY()
	TObjectPtr<const UGUIS_UIAction> CurrentAction{nullptr};

	UPROPERTY()
	TObjectPtr<UGUIS_AsyncAction_ShowModel> ModalTask{nullptr};
};
