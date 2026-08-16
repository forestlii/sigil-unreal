// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/Modal/SigilGameModal.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "UI/Foundation/SigilButtonBase.h"
#include "UI/Modal/SigilGameModalTypes.h"
#include "SigilUILogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilGameModal)

#define LOCTEXT_NAMESPACE "SigilGameModal"

USigilGameModalWidget::USigilGameModalWidget()
{
	bIsModal = true;
}

void USigilGameModalWidget::SetupModal(const USigilModalDefinition* ModalDefinition, FSigilModalActionResultSignature ModalActionCallback)
{
	OnModalActionCallback = ModalActionCallback;

	if (!ModalDefinition)
	{
		UE_LOG(LogSigilUI, Error, TEXT("[%s] SetupModal called with a null modal definition."), *GetName());
		return;
	}
	if (!EntryBox_Buttons || !Text_Header || !Text_Body)
	{
		UE_LOG(LogSigilUI, Error, TEXT("[%s] modal widget is missing a bound sub-widget (EntryBox_Buttons / Text_Header / Text_Body)."), *GetName());
		return;
	}

	EntryBox_Buttons->Reset<USigilButtonBase>([](USigilButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	Text_Header->SetText(ModalDefinition->Header);
	Text_Body->SetText(ModalDefinition->Body);

	for (const auto& Pair : ModalDefinition->ModalActions)
	{
		TSubclassOf<USigilButtonBase> ButtonClass = !Pair.Value.ButtonType.IsNull() ? Pair.Value.ButtonType.LoadSynchronous() : nullptr;
		USigilButtonBase* Button = EntryBox_Buttons->CreateEntry<USigilButtonBase>(ButtonClass);
		if (!Button)
		{
			// CreateEntry returns null when the class failed to load, is not a USigilButtonBase, or fails the entry box's class check.
			UE_LOG(LogSigilUI, Error, TEXT("[%s] failed to create a modal button for action '%s' (button class '%s'); action skipped."),
			       *GetName(), *Pair.Key.ToString(), *Pair.Value.ButtonType.ToString());
			continue;
		}
		Button->SetTriggeringInputAction(Pair.Value.InputAction);
		Button->OnClicked().AddUObject(this, &ThisClass::CloseModal, Pair.Key);
		if (!Pair.Value.DisplayText.IsEmpty())
		{
			Button->SetButtonText(Pair.Value.DisplayText);
		}
	}

	OnSetupModal(ModalDefinition);
}

void USigilGameModalWidget::CloseModal(FGameplayTag ModalActionResult)
{
	DeactivateWidget();
	OnModalActionCallback.ExecuteIfBound(ModalActionResult);
}

void USigilGameModalWidget::KillModal()
{
}

#undef LOCTEXT_NAMESPACE
