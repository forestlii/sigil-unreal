// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Actions/GUIS_UIActionWidget.h"

#include "GUIS_LogChannels.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/GUIS_GameplayTags.h"
#include "UI/Actions/GUIS_AsyncAction_ShowModel.h"
#include "UI/Actions/GUIS_UIActionFactory.h"

void UGUIS_UIActionWidget::SetAssociatedData(UObject* Data)
{
	if (Data == nullptr)
	{
		UnregisterActions();
	}
	AssociatedData = Data;
}

void UGUIS_UIActionWidget::RegisterActions()
{
	if (!AssociatedData.IsValid())
	{
		return;
	}

	if (!IsValid(ActionFactory))
	{
		return;
	}

	TArray<UGUIS_UIAction*> Actions = ActionFactory->FindAvailableUIActionsForData(AssociatedData.Get());

	for (const UGUIS_UIAction* Action : Actions)
	{
		if (Action->CanInvoke(AssociatedData.Get(), GetOwningPlayer()))
		{
			FBindUIActionArgs BindArgs(Action->GetInputActionData(), Action->GetShouldDisplayInActionBar(),
			                           FSimpleDelegate::CreateLambda([this,Action]()
			                           {
				                           HandleUIAction(Action);
			                           }));

			ActionBindings.Add(RegisterUIActionBinding(BindArgs));
		}
	}
}

void UGUIS_UIActionWidget::RegisterActionsWithFactory(TSoftObjectPtr<UGUIS_UIActionFactory> InActionFactory)
{
	if (InActionFactory.IsNull())
	{
		UE_LOG(LogGUIS, Warning, TEXT("Passed invalid action factory!"))
		return;
	}

	UGUIS_UIActionFactory* Factory = InActionFactory.LoadSynchronous();

	if (Factory == nullptr)
	{
		UE_LOG(LogGUIS, Warning, TEXT("Failed to load action factory!"))
		return;
	}

	ActionFactory = Factory;

	RegisterActions();
}

void UGUIS_UIActionWidget::UnregisterActions()
{
	for (FUIActionBindingHandle& ActionBinding : ActionBindings)
	{
		ActionBinding.Unregister();
	}

	ActionBindings.Empty();
	CancelAction();
}

void UGUIS_UIActionWidget::CancelAction()
{
	if (ModalTask)
	{
		ModalTask->OnModalAction.RemoveDynamic(this, &ThisClass::HandleModalAction);
		ModalTask->Cancel();
		ModalTask = nullptr;
	}
	CurrentAction = nullptr;
}

#if WITH_EDITOR
const FText UGUIS_UIActionWidget::GetPaletteCategory()
{
	return FText::FromString(TEXT("Generic UI"));
}
#endif

void UGUIS_UIActionWidget::HandleUIAction(const UGUIS_UIAction* Action)
{
	if (ModalTask && ModalTask->IsActive())
	{
		return;
	}
	if (AssociatedData.IsValid())
	{
		if (Action->GetRequiresConfirmation() && !Action->GetConfirmationModalClass().IsNull())
		{
			ModalTask = UGUIS_AsyncAction_ShowModel::ShowModal(GetWorld(), Action->GetConfirmationModalClass());
			CurrentAction = Action;
			ModalTask->OnModalAction.AddDynamic(this, &ThisClass::HandleModalAction);
			ModalTask->Activate();
		}
		else
		{
			if (Action->CanInvoke(AssociatedData.Get(), GetOwningPlayer()))
			{
				Action->InvokeAction(AssociatedData.Get(), GetOwningPlayer());
			}
		}
	}
}

void UGUIS_UIActionWidget::HandleModalAction(FGameplayTag ActionTag)
{
	if (ActionTag == GUIS_GameModalActionTags::Yes || ActionTag == GUIS_GameModalActionTags::Ok)
	{
		if (CurrentAction && CurrentAction->CanInvoke(AssociatedData.Get(), GetOwningPlayer()))
		{
			CurrentAction->InvokeAction(AssociatedData.Get(), GetOwningPlayer());
		}
		CancelAction();
	}
	if (ActionTag == GUIS_GameModalActionTags::No)
	{
		CancelAction();
	}
}
