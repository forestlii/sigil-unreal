// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilDialogueEditorTestTypes.h"

int32 USigilDialogueEditorTestCondition::CallCount = 0;
int32 USigilDialogueEditorTestEvent::CallCount = 0;

bool USigilDialogueEditorTestCondition::Evaluate_Implementation(
	const FSigilNarrativeContext& Context) const
{
	(void)Context;
	++CallCount;
	return true;
}

void USigilDialogueEditorTestEvent::Execute_Implementation(
	const FSigilNarrativeContext& Context)
{
	(void)Context;
	++CallCount;
}
