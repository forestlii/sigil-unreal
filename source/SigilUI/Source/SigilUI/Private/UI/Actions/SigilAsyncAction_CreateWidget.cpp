// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/Actions/SigilAsyncAction_CreateWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "UI/SigilGameUIFunctionLibrary.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAsyncAction_CreateWidget)

class UUserWidget;

static const FName InputFilterReason_Template = FName(TEXT("CreatingWidgetAsync"));

USigilAsyncAction_CreateWidget::USigilAsyncAction_CreateWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , bSuspendInputUntilComplete(true)
{
}

USigilAsyncAction_CreateWidget* USigilAsyncAction_CreateWidget::CreateWidgetAsync(UObject* InWorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer,
                                                                                bool bSuspendInputUntilComplete)
{
	if (InUserWidgetSoftClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null UserWidgetSoftClass"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	USigilAsyncAction_CreateWidget* Action = NewObject<USigilAsyncAction_CreateWidget>();
	Action->UserWidgetSoftClass = InUserWidgetSoftClass;
	Action->OwningPlayer = InOwningPlayer;
	Action->World = World;
	Action->GameInstance = World->GetGameInstance();
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void USigilAsyncAction_CreateWidget::Activate()
{
	SuspendInputToken = bSuspendInputUntilComplete ? USigilGameUIFunctionLibrary::SuspendInputForPlayer(OwningPlayer.Get(), InputFilterReason_Template) : NAME_None;

	TWeakObjectPtr<USigilAsyncAction_CreateWidget> LocalWeakThis(this);
	StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		UserWidgetSoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnWidgetLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	// Setup a cancel delegate so that we can resume input if this handler is canceled.
	StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
	                                                                          [this]()
	                                                                          {
		                                                                          USigilGameUIFunctionLibrary::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	                                                                          })
	);
}

void USigilAsyncAction_CreateWidget::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}

void USigilAsyncAction_CreateWidget::OnWidgetLoaded()
{
	if (bSuspendInputUntilComplete)
	{
		USigilGameUIFunctionLibrary::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	}

	// If the load as successful, create it, otherwise don't complete this.
	TSubclassOf<UUserWidget> UserWidgetClass = UserWidgetSoftClass.Get();
	if (UserWidgetClass)
	{
		UUserWidget* UserWidget = UWidgetBlueprintLibrary::Create(World.Get(), UserWidgetClass, OwningPlayer.Get());
		OnComplete.Broadcast(UserWidget);
	}

	StreamingHandle.Reset();

	SetReadyToDestroy();
}
