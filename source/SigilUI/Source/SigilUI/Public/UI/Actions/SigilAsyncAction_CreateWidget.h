// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/SoftObjectPtr.h"

#include "SigilAsyncAction_CreateWidget.generated.h"

class APlayerController;
class UGameInstance;
class UUserWidget;
class UWorld;
struct FFrame;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSigilCreateWidgetSignature, UUserWidget *, UserWidget);

/**
 * Load the widget class asynchronously, the instance the widget after the loading completes, and return it on OnComplete.
 */
UCLASS(BlueprintType)
class SIGILUI_API USigilAsyncAction_CreateWidget : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Cancel() override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static USigilAsyncAction_CreateWidget *CreateWidgetAsync(UObject *WorldContextObject, TSoftClassPtr<UUserWidget> UserWidgetSoftClass, APlayerController *OwningPlayer,
															 bool bSuspendInputUntilComplete = true);

	virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FSigilCreateWidgetSignature OnComplete;

private:
	void OnWidgetLoaded();

	FName SuspendInputToken;
	TWeakObjectPtr<APlayerController> OwningPlayer;
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<UGameInstance> GameInstance;
	bool bSuspendInputUntilComplete;
	TSoftClassPtr<UUserWidget> UserWidgetSoftClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;
};
