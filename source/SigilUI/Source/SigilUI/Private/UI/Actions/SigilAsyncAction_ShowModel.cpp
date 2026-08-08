// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/Actions/SigilAsyncAction_ShowModel.h"
#include "SigilSigilUISettings.h"
#include "Engine/GameInstance.h"
#include "UI/SigilGameUIFunctionLibrary.h"
#include "UI/SigilGameUILayout.h"
#include "UI/SigilUITags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAsyncAction_ShowModel)

USigilAsyncAction_ShowModel::USigilAsyncAction_ShowModel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

USigilAsyncAction_ShowModel* USigilAsyncAction_ShowModel::ShowModal(UObject* InWorldContextObject, TSoftClassPtr<USigilModalDefinition> ModalDefinition)
{
	if (ModalDefinition.IsNull())
	{
		return nullptr;
	}

	ModalDefinition.LoadSynchronous();
	
	const USigilModalDefinition* Modal = ModalDefinition->GetDefaultObject<USigilModalDefinition>();
	if (Modal == nullptr)
		return nullptr;

	if (Modal->ModalWidget.IsNull())
		return nullptr;

	const TSubclassOf<USigilGameModalWidget> ModalWidgetClass = Modal->ModalWidget.LoadSynchronous();
	if (ModalWidgetClass == nullptr)
		return nullptr;

	USigilAsyncAction_ShowModel* Action = NewObject<USigilAsyncAction_ShowModel>();
	Action->ModalWidgetClass = ModalWidgetClass;
	Action->WorldContextObject = InWorldContextObject;
	Action->ModalDefinition = Modal;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void USigilAsyncAction_ShowModel::Activate()
{
	if (WorldContextObject && !TargetPlayerController)
	{
		if (UUserWidget* UserWidget = Cast<UUserWidget>(WorldContextObject))
		{
			TargetPlayerController = UserWidget->GetOwningPlayer<APlayerController>();
		}
		else if (APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			TargetPlayerController = PC;
		}
		else if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance<UGameInstance>())
			{
				TargetPlayerController = GameInstance->GetPrimaryPlayerController(false);
			}
		}
	}

	if (TargetPlayerController)
	{
		if (USigilGameUILayout* Layout = USigilGameUIFunctionLibrary::GetGameUILayoutForPlayer(TargetPlayerController))
		{
			FSigilModalActionResultSignature ResultCallback = FSigilModalActionResultSignature::CreateUObject(this, &USigilAsyncAction_ShowModel::HandleModalAction);
			const USigilModalDefinition* TempDescriptor = ModalDefinition;
			Layout->PushWidgetToLayerStack<USigilGameModalWidget>(SigilGameUILayerTags::Modal, ModalWidgetClass, [TempDescriptor, ResultCallback](USigilGameModalWidget& ModalInstance)
			{
				ModalInstance.SetupModal(TempDescriptor, ResultCallback);
			});
			return;
		}
	}

	// If we couldn't make the confirmation, just handle an unknown result and broadcast nothing
	HandleModalAction(SigilGameModalActionTags::Unknown);
}


void USigilAsyncAction_ShowModel::HandleModalAction(FGameplayTag ModalActionTag)
{
	OnModalAction.Broadcast(ModalActionTag);
	SetReadyToDestroy();
}
