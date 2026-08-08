// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/Actions/SigilAsyncAction_PushContentToUILayer.h"
#include "Engine/Engine.h"
#include "UI/SigilGameUILayout.h"
#include "UObject/Stack.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAsyncAction_PushContentToUILayer)

USigilAsyncAction_PushContentToUILayer::USigilAsyncAction_PushContentToUILayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

USigilAsyncAction_PushContentToUILayer* USigilAsyncAction_PushContentToUILayer::PushContentToUILayer(USigilGameUILayout* UILayout,
                                                                                                     TSoftClassPtr<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerName,
                                                                                                     bool bSuspendInputUntilComplete)
{
	if (!IsValid(UILayout))
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayer was passed a invalid Layout"), ELogVerbosity::Error);
		return nullptr;
	}
	if (InWidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayer was passed a null WidgetClass"), ELogVerbosity::Error);
		return nullptr;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(UILayout->GetWorld(), EGetWorldErrorMode::LogAndReturnNull))
	{
		USigilAsyncAction_PushContentToUILayer* Action = NewObject<USigilAsyncAction_PushContentToUILayer>();
		Action->WidgetClass = InWidgetClass;
		Action->RootLayout = UILayout;
		Action->OwningPlayerPtr = UILayout->GetOwningPlayer();
		Action->LayerName = InLayerName;
		Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
		Action->RegisterWithGameInstance(World);

		return Action;
	}

	return nullptr;
}

USigilAsyncAction_PushContentToUILayer* USigilAsyncAction_PushContentToUILayer::PushContentToUILayerForPlayer(APlayerController* PlayerController,
                                                                                                              TSoftClassPtr<UCommonActivatableWidget> InWidgetClass,
                                                                                                              FGameplayTag InLayerName, bool bSuspendInputUntilComplete)
{
	if (!IsValid(PlayerController))
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayerForPlayer was passed a invalid PlayerController"), ELogVerbosity::Error);
		return nullptr;
	}
	if (InWidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayer was passed a null WidgetClass"), ELogVerbosity::Error);
		return nullptr;
	}

	USigilGameUILayout* UILayout = USigilGameUIFunctionLibrary::GetGameUILayoutForPlayer(PlayerController);

	if (UILayout == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayerForPlayer failed to find UILayout for player."), ELogVerbosity::Error);
		return nullptr;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(UILayout->GetWorld(), EGetWorldErrorMode::LogAndReturnNull))
	{
		USigilAsyncAction_PushContentToUILayer* Action = NewObject<USigilAsyncAction_PushContentToUILayer>();
		Action->WidgetClass = InWidgetClass;
		Action->RootLayout = UILayout;
		Action->OwningPlayerPtr = PlayerController;
		Action->LayerName = InLayerName;
		Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
		Action->RegisterWithGameInstance(World);

		return Action;
	}

	return nullptr;
}


void USigilAsyncAction_PushContentToUILayer::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}

void USigilAsyncAction_PushContentToUILayer::Activate()
{
	if (RootLayout.IsValid())
	{
		TWeakObjectPtr<USigilAsyncAction_PushContentToUILayer> WeakThis = this;
		StreamingHandle = RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(LayerName, bSuspendInputUntilComplete, WidgetClass,
		                                                                                    [this, WeakThis](ESigilAsyncWidgetLayerState State, UCommonActivatableWidget* Widget)
		                                                                                    {
			                                                                                    if (WeakThis.IsValid())
			                                                                                    {
				                                                                                    switch (State)
				                                                                                    {
				                                                                                    case ESigilAsyncWidgetLayerState::Initialize:
					                                                                                    BeforePush.Broadcast(Widget);
					                                                                                    break;
				                                                                                    case ESigilAsyncWidgetLayerState::AfterPush:
					                                                                                    AfterPush.Broadcast(Widget);
					                                                                                    SetReadyToDestroy();
					                                                                                    break;
				                                                                                    case ESigilAsyncWidgetLayerState::Canceled:
					                                                                                    SetReadyToDestroy();
					                                                                                    break;
				                                                                                    }
			                                                                                    }
			                                                                                    SetReadyToDestroy();
		                                                                                    });
	}
	else
	{
		SetReadyToDestroy();
	}
}
