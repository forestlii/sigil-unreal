// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/Actions/GUIS_AsyncAction_ShowModel.h"
#include "GUIS_GenericUISystemSettings.h"
#include "Engine/GameInstance.h"
#include "UI/GUIS_GameUIFunctionLibrary.h"
#include "UI/GUIS_GameUILayout.h"
#include "UI/GUIS_GameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_AsyncAction_ShowModel)

UGUIS_AsyncAction_ShowModel::UGUIS_AsyncAction_ShowModel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// UGUIS_AsyncAction_ShowModel* UGUIS_AsyncAction_ShowModel::ShowModal(UObject* InWorldContextObject, FGameplayTag ModalTag, UGUIS_ModalDefinition* ModalDefinition)
// {
// 	const UGUIS_GameUIData* UIData = UGUIS_GenericUISystemSettings::GetGameUIData();
// 	if (!UIData)
// 		return nullptr;
//
// 	const TSoftClassPtr<UGUIS_GameModalWidget> SoftModalWidgetClass = UIData->FindWidgetClassForModal(ModalTag);
// 	if (SoftModalWidgetClass.IsNull())
// 		return nullptr;
// 	const TSubclassOf<UGUIS_GameModalWidget> ModalWidgetClass = SoftModalWidgetClass.LoadSynchronous();
// 	if (ModalWidgetClass == nullptr)
// 		return nullptr;
//
// 	UGUIS_AsyncAction_ShowModel* Action = NewObject<UGUIS_AsyncAction_ShowModel>();
// 	Action->ModalWidgetClass = ModalWidgetClass;
// 	Action->WorldContextObject = InWorldContextObject;
// 	Action->ModalDefinition = ModalDefinition;
// 	Action->RegisterWithGameInstance(InWorldContextObject);
//
// 	return Action;
// }

UGUIS_AsyncAction_ShowModel* UGUIS_AsyncAction_ShowModel::ShowModal(UObject* InWorldContextObject, TSoftClassPtr<UGUIS_ModalDefinition> ModalDefinition)
{
	if (ModalDefinition.IsNull())
	{
		return nullptr;
	}

	ModalDefinition.LoadSynchronous();
	
	const UGUIS_ModalDefinition* Modal = ModalDefinition->GetDefaultObject<UGUIS_ModalDefinition>();
	if (Modal == nullptr)
		return nullptr;

	if (Modal->ModalWidget.IsNull())
		return nullptr;

	const TSubclassOf<UGUIS_GameModalWidget> ModalWidgetClass = Modal->ModalWidget.LoadSynchronous();
	if (ModalWidgetClass == nullptr)
		return nullptr;

	UGUIS_AsyncAction_ShowModel* Action = NewObject<UGUIS_AsyncAction_ShowModel>();
	Action->ModalWidgetClass = ModalWidgetClass;
	Action->WorldContextObject = InWorldContextObject;
	Action->ModalDefinition = Modal;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void UGUIS_AsyncAction_ShowModel::Activate()
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
		if (UGUIS_GameUILayout* Layout = UGUIS_GameUIFunctionLibrary::GetGameUILayoutForPlayer(TargetPlayerController))
		{
			FGUIS_ModalActionResultSignature ResultCallback = FGUIS_ModalActionResultSignature::CreateUObject(this, &UGUIS_AsyncAction_ShowModel::HandleModalAction);
			const UGUIS_ModalDefinition* TempDescriptor = ModalDefinition;
			Layout->PushWidgetToLayerStack<UGUIS_GameModalWidget>(GUIS_GameUILayerTags::Modal, ModalWidgetClass, [TempDescriptor, ResultCallback](UGUIS_GameModalWidget& ModalInstance)
			{
				ModalInstance.SetupModal(TempDescriptor, ResultCallback);
			});
		}
	}

	// If we couldn't make the confirmation, just handle an unknown result and broadcast nothing
	HandleModalAction(GUIS_GameModalActionTags::Unknown);
}


void UGUIS_AsyncAction_ShowModel::HandleModalAction(FGameplayTag ModalActionTag)
{
	OnModalAction.Broadcast(ModalActionTag);
	SetReadyToDestroy();
}
