// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/Actions/GUIS_AsyncAction_CreateWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "UI/GUIS_GameUIFunctionLibrary.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_AsyncAction_CreateWidget)

class UUserWidget;

static const FName InputFilterReason_Template = FName(TEXT("CreatingWidgetAsync"));

UGUIS_AsyncAction_CreateWidget::UGUIS_AsyncAction_CreateWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , bSuspendInputUntilComplete(true)
{
}

UGUIS_AsyncAction_CreateWidget* UGUIS_AsyncAction_CreateWidget::CreateWidgetAsync(UObject* InWorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer,
                                                                                bool bSuspendInputUntilComplete)
{
	if (InUserWidgetSoftClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null UserWidgetSoftClass"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	UGUIS_AsyncAction_CreateWidget* Action = NewObject<UGUIS_AsyncAction_CreateWidget>();
	Action->UserWidgetSoftClass = InUserWidgetSoftClass;
	Action->OwningPlayer = InOwningPlayer;
	Action->World = World;
	Action->GameInstance = World->GetGameInstance();
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UGUIS_AsyncAction_CreateWidget::Activate()
{
	SuspendInputToken = bSuspendInputUntilComplete ? UGUIS_GameUIFunctionLibrary::SuspendInputForPlayer(OwningPlayer.Get(), InputFilterReason_Template) : NAME_None;

	TWeakObjectPtr<UGUIS_AsyncAction_CreateWidget> LocalWeakThis(this);
	StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		UserWidgetSoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnWidgetLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	// Setup a cancel delegate so that we can resume input if this handler is canceled.
	StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
	                                                                          [this]()
	                                                                          {
		                                                                          UGUIS_GameUIFunctionLibrary::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	                                                                          })
	);
}

void UGUIS_AsyncAction_CreateWidget::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}

void UGUIS_AsyncAction_CreateWidget::OnWidgetLoaded()
{
	if (bSuspendInputUntilComplete)
	{
		UGUIS_GameUIFunctionLibrary::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
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
