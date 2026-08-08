// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "UI/Modal/GUIS_GameModal.h"
#include "UObject/ObjectPtr.h"
#include "GUIS_AsyncAction_ShowModel.generated.h"

enum class EGGF_DialogMessagingResult : uint8;
class FText;
class ULocalPlayer;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGUIS_ModalResultSignature, FGameplayTag, Result);

/**
 * Allows easily triggering an async confirmation dialog in blueprints that you can then wait on the result.
 */
UCLASS()
class UGUIS_AsyncAction_ShowModel : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	// UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	// static UGUIS_AsyncAction_ShowModel* ShowModal(UObject* InWorldContextObject, FGameplayTag ModalTag, UGUIS_ModalDefinition* ModalDefinition);

	/**
	 * 给定一个Modal定义，然后显示该Modal。 
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UGUIS_AsyncAction_ShowModel* ShowModal(UObject* InWorldContextObject, TSoftClassPtr<UGUIS_ModalDefinition> ModalDefinition);

	virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FGUIS_ModalResultSignature OnModalAction;

private:
	void HandleModalAction(FGameplayTag ModalActionTag);


	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	// UPROPERTY(Transient)
	// TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> TargetPlayerController;

	UPROPERTY(Transient)
	TSubclassOf<UGUIS_GameModalWidget> ModalWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<const UGUIS_ModalDefinition> ModalDefinition;
};
