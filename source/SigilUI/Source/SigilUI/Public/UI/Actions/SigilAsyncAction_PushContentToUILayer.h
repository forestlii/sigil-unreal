// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

#include "SigilAsyncAction_PushContentToUILayer.generated.h"

class USigilGameUILayout;
class APlayerController;
class UCommonActivatableWidget;
class UObject;
struct FFrame;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushContentToLayerForPlayerAsyncDelegate, UCommonActivatableWidget *, UserWidget);

/**
 *
 */
UCLASS(BlueprintType)
class SIGILUI_API USigilAsyncAction_PushContentToUILayer : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()
	virtual void Cancel() override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static USigilAsyncAction_PushContentToUILayer* PushContentToUILayer(USigilGameUILayout* UILayout,
	                                                                    UPARAM(meta = (AllowAbstract = false))
	                                                                    TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
	                                                                    UPARAM(meta = (Categories = "Sigil.UI.Layer"))
	                                                                    FGameplayTag LayerName, bool bSuspendInputUntilComplete = true);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static USigilAsyncAction_PushContentToUILayer* PushContentToUILayerForPlayer(APlayerController* PlayerController,
	                                                                             UPARAM(meta = (AllowAbstract = false))
	                                                                             TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
	                                                                             UPARAM(meta = (Categories = "Sigil.UI.Layer"))
	                                                                             FGameplayTag LayerName, bool bSuspendInputUntilComplete = true);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FPushContentToLayerForPlayerAsyncDelegate BeforePush;

	UPROPERTY(BlueprintAssignable)
	FPushContentToLayerForPlayerAsyncDelegate AfterPush;

private:
	FGameplayTag LayerName;
	bool bSuspendInputUntilComplete = false;
	TWeakObjectPtr<APlayerController> OwningPlayerPtr;
	TWeakObjectPtr<USigilGameUILayout> RootLayout;
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;

	TSharedPtr<FStreamableHandle> StreamingHandle;
};
