// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "UI/Modal/SigilGameModal.h"
#include "UObject/ObjectPtr.h"
#include "SigilAsyncAction_ShowModel.generated.h"

enum class EGGF_DialogMessagingResult : uint8;
class FText;
class ULocalPlayer;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilModalResultSignature, FGameplayTag, Result);

/**
 * Allows easily triggering an async confirmation dialog in blueprints that you can then wait on the result.
 */
UCLASS()
class USigilAsyncAction_ShowModel : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * 给定一个Modal定义，然后显示该Modal。 
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static USigilAsyncAction_ShowModel* ShowModal(UObject* InWorldContextObject, TSoftClassPtr<USigilModalDefinition> ModalDefinition);

	virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FSigilModalResultSignature OnModalAction;

private:
	void HandleModalAction(FGameplayTag ModalActionTag);


	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	// UPROPERTY(Transient)
	// TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> TargetPlayerController;

	UPROPERTY(Transient)
	TSubclassOf<USigilGameModalWidget> ModalWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<const USigilModalDefinition> ModalDefinition;
};
