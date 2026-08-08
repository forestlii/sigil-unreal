// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/Modal/SigilGameModal.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "UI/Foundation/SigilButtonBase.h"
#include "UI/Modal/SigilGameModalTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilGameModal)

#define LOCTEXT_NAMESPACE "SigilGameModal"

USigilGameModalWidget::USigilGameModalWidget()
{
	bIsModal = true;
}

void USigilGameModalWidget::SetupModal(const USigilModalDefinition* ModalDefinition, FSigilModalActionResultSignature ModalActionCallback)
{
	OnModalActionCallback = ModalActionCallback;

	EntryBox_Buttons->Reset<USigilButtonBase>([](USigilButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	Text_Header->SetText(ModalDefinition->Header);
	Text_Body->SetText(ModalDefinition->Body);

	for (const auto& Pair : ModalDefinition->ModalActions)
	{
		USigilButtonBase* Button = EntryBox_Buttons->CreateEntry<USigilButtonBase>(!Pair.Value.ButtonType.IsNull() ? Pair.Value.ButtonType.LoadSynchronous() : nullptr);
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
