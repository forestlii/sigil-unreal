// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/SoftObjectPtr.h"

#include "GUIS_AsyncAction_CreateWidget.generated.h"

class APlayerController;
class UGameInstance;
class UUserWidget;
class UWorld;
struct FFrame;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGUIS_CreateWidgetSignature, UUserWidget *, UserWidget);

/**
 * Load the widget class asynchronously, the instance the widget after the loading completes, and return it on OnComplete.
 */
UCLASS(BlueprintType)
class GENERICUISYSTEM_API UGUIS_AsyncAction_CreateWidget : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Cancel() override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GUIS", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UGUIS_AsyncAction_CreateWidget *CreateWidgetAsync(UObject *WorldContextObject, TSoftClassPtr<UUserWidget> UserWidgetSoftClass, APlayerController *OwningPlayer,
															 bool bSuspendInputUntilComplete = true);

	virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FGUIS_CreateWidgetSignature OnComplete;

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
