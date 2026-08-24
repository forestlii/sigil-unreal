// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNarrativeSetFlagEvent.h"

#include "SigilNarrativeSubsystem.h"

void USigilNarrativeSetFlagEvent::Execute_Implementation(const FSigilNarrativeContext& Context)
{
	if (Context.NarrativeSubsystem)
	{
		Context.NarrativeSubsystem->SetFlag(Flag, bEnabled);
	}
}
