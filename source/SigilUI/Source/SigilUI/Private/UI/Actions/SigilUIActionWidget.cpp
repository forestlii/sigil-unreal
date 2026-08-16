// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Actions/SigilUIActionWidget.h"

#include "SigilUILogChannels.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/SigilUITags.h"
#include "UI/Actions/SigilAsyncAction_ShowModel.h"
#include "UI/Actions/SigilUIActionFactory.h"

void USigilUIActionWidget::SetAssociatedData(UObject* Data)
{
	if (Data == nullptr)
	{
		UnregisterActions();
	}
	AssociatedData = Data;
}

void USigilUIActionWidget::RegisterActions()
{
	if (!AssociatedData.IsValid())
	{
		return;
	}

	if (!IsValid(ActionFactory))
	{
		return;
	}

	TArray<USigilUIAction*> Actions = ActionFactory->FindAvailableUIActionsForData(AssociatedData.Get());

	for (const USigilUIAction* Action : Actions)
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

void USigilUIActionWidget::RegisterActionsWithFactory(TSoftObjectPtr<USigilUIActionFactory> InActionFactory)
{
	if (InActionFactory.IsNull())
	{
		UE_LOG(LogSigilUI, Warning, TEXT("Passed invalid action factory!"))
		return;
	}

	USigilUIActionFactory* Factory = InActionFactory.LoadSynchronous();

	if (Factory == nullptr)
	{
		UE_LOG(LogSigilUI, Warning, TEXT("Failed to load action factory!"))
		return;
	}

	ActionFactory = Factory;

	RegisterActions();
}

void USigilUIActionWidget::UnregisterActions()
{
	for (FUIActionBindingHandle& ActionBinding : ActionBindings)
	{
		ActionBinding.Unregister();
	}

	ActionBindings.Empty();
	CancelAction();
}

void USigilUIActionWidget::CancelAction()
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
const FText USigilUIActionWidget::GetPaletteCategory()
{
	return FText::FromString(TEXT("Generic UI"));
}
#endif

void USigilUIActionWidget::HandleUIAction(const USigilUIAction* Action)
{
	if (ModalTask && ModalTask->IsActive())
	{
		return;
	}
	if (AssociatedData.IsValid())
	{
		if (Action->GetRequiresConfirmation() && !Action->GetConfirmationModalClass().IsNull())
		{
			ModalTask = USigilAsyncAction_ShowModel::ShowModal(GetWorld(), Action->GetConfirmationModalClass());
			if (!ModalTask)
			{
				// Confirmation modal could not be created (bad soft class / widget): fail closed, do not invoke the action.
				UE_LOG(LogSigilUI, Error, TEXT("[%s] could not create the confirmation modal for action '%s'; the action was not invoked."), *GetName(), *Action->GetActionID().ToString());
				return;
			}
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

void USigilUIActionWidget::HandleModalAction(FGameplayTag ActionTag)
{
	if (ActionTag == SigilGameModalActionTags::Yes || ActionTag == SigilGameModalActionTags::Ok)
	{
		if (CurrentAction && CurrentAction->CanInvoke(AssociatedData.Get(), GetOwningPlayer()))
		{
			CurrentAction->InvokeAction(AssociatedData.Get(), GetOwningPlayer());
		}
	}

	// Any other result (No / Cancel / Unknown) just cancels; always clear the pending action and task.
	CancelAction();
}
