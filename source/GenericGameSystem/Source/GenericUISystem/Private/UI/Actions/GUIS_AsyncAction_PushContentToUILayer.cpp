// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/Actions/GUIS_AsyncAction_PushContentToUILayer.h"
#include "Engine/Engine.h"
#include "UI/GUIS_GameUILayout.h"
#include "UObject/Stack.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_AsyncAction_PushContentToUILayer)

UGUIS_AsyncAction_PushContentToUILayer::UGUIS_AsyncAction_PushContentToUILayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGUIS_AsyncAction_PushContentToUILayer* UGUIS_AsyncAction_PushContentToUILayer::PushContentToUILayer(UGUIS_GameUILayout* UILayout,
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
		UGUIS_AsyncAction_PushContentToUILayer* Action = NewObject<UGUIS_AsyncAction_PushContentToUILayer>();
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

UGUIS_AsyncAction_PushContentToUILayer* UGUIS_AsyncAction_PushContentToUILayer::PushContentToUILayerForPlayer(APlayerController* PlayerController,
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

	UGUIS_GameUILayout* UILayout = UGUIS_GameUIFunctionLibrary::GetGameUILayoutForPlayer(PlayerController);

	if (UILayout == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayerForPlayer failed to find UILayout for player."), ELogVerbosity::Error);
		return nullptr;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(UILayout->GetWorld(), EGetWorldErrorMode::LogAndReturnNull))
	{
		UGUIS_AsyncAction_PushContentToUILayer* Action = NewObject<UGUIS_AsyncAction_PushContentToUILayer>();
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


void UGUIS_AsyncAction_PushContentToUILayer::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}

void UGUIS_AsyncAction_PushContentToUILayer::Activate()
{
	if (RootLayout.IsValid())
	{
		TWeakObjectPtr<UGUIS_AsyncAction_PushContentToUILayer> WeakThis = this;
		StreamingHandle = RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(LayerName, bSuspendInputUntilComplete, WidgetClass,
		                                                                                    [this, WeakThis](EGUIS_AsyncWidgetLayerState State, UCommonActivatableWidget* Widget)
		                                                                                    {
			                                                                                    if (WeakThis.IsValid())
			                                                                                    {
				                                                                                    switch (State)
				                                                                                    {
				                                                                                    case EGUIS_AsyncWidgetLayerState::Initialize:
					                                                                                    BeforePush.Broadcast(Widget);
					                                                                                    break;
				                                                                                    case EGUIS_AsyncWidgetLayerState::AfterPush:
					                                                                                    AfterPush.Broadcast(Widget);
					                                                                                    SetReadyToDestroy();
					                                                                                    break;
				                                                                                    case EGUIS_AsyncWidgetLayerState::Canceled:
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
