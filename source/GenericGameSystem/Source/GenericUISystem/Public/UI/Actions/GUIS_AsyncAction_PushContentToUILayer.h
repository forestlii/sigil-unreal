// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

#include "GUIS_AsyncAction_PushContentToUILayer.generated.h"

class UGUIS_GameUILayout;
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
class GENERICUISYSTEM_API UGUIS_AsyncAction_PushContentToUILayer : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()
	virtual void Cancel() override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGUIS_AsyncAction_PushContentToUILayer* PushContentToUILayer(UGUIS_GameUILayout* UILayout,
	                                                                    UPARAM(meta = (AllowAbstract = false))
	                                                                    TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
	                                                                    UPARAM(meta = (Categories = "UI.Layer,GUIS.Layer"))
	                                                                    FGameplayTag LayerName, bool bSuspendInputUntilComplete = true);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGUIS_AsyncAction_PushContentToUILayer* PushContentToUILayerForPlayer(APlayerController* PlayerController,
	                                                                             UPARAM(meta = (AllowAbstract = false))
	                                                                             TSoftClassPtr<UCommonActivatableWidget> WidgetClass,
	                                                                             UPARAM(meta = (Categories = "UI.Layer,GUIS.Layer"))
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
	TWeakObjectPtr<UGUIS_GameUILayout> RootLayout;
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;

	TSharedPtr<FStreamableHandle> StreamingHandle;
};
