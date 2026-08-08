// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Foundation/SigilButtonBase.h"

#include "CommonActionWidget.h"


void USigilButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	OnUpdateButtonStyle();
	RefreshButtonText();
}

void USigilButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	OnUpdateButtonStyle();
	RefreshButtonText();
}

void USigilButtonBase::SetButtonText(const FText& InText)
{
	bOverride_ButtonText = !InText.IsEmpty();
	ButtonText = InText;
	RefreshButtonText();
}

void USigilButtonBase::RefreshButtonText()
{
	if (!bOverride_ButtonText || ButtonText.IsEmpty())
	{
		if (InputActionWidget)
		{
			const FText ActionDisplayText = InputActionWidget->GetDisplayText();
			if (!ActionDisplayText.IsEmpty())
			{
				OnUpdateButtonText(ActionDisplayText);
				return;
			}
		}
	}

	OnUpdateButtonText(ButtonText);
}


void USigilButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);

	OnUpdateButtonStyle();
}

#if WITH_EDITOR
const FText USigilButtonBase::GetPaletteCategory()
{
	return PaletteCategory;
}
#endif
