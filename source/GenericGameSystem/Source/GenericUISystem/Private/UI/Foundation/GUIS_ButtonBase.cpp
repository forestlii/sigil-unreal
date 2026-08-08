// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Foundation/GUIS_ButtonBase.h"

#include "CommonActionWidget.h"


void UGUIS_ButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	OnUpdateButtonStyle();
	RefreshButtonText();
}

void UGUIS_ButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	OnUpdateButtonStyle();
	RefreshButtonText();
}

void UGUIS_ButtonBase::SetButtonText(const FText& InText)
{
	bOverride_ButtonText = !InText.IsEmpty();
	ButtonText = InText;
	RefreshButtonText();
}

void UGUIS_ButtonBase::RefreshButtonText()
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


void UGUIS_ButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);

	OnUpdateButtonStyle();
}

#if WITH_EDITOR
const FText UGUIS_ButtonBase::GetPaletteCategory()
{
	return PaletteCategory;
}
#endif
