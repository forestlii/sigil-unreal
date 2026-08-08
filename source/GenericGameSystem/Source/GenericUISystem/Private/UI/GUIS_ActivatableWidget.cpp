// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/GUIS_ActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_ActivatableWidget)

#define LOCTEXT_NAMESPACE "GGF"

UGUIS_ActivatableWidget::UGUIS_ActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGUIS_ActivatableWidget::SetIsBackHandler(bool bNewState)
{
}

TOptional<FUIInputConfig> UGUIS_ActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EGUIS_ActivatableWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EGUIS_ActivatableWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EGUIS_ActivatableWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EGUIS_ActivatableWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR

void UGUIS_ActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UGUIS_ActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note",
			                        "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE
