// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNarrativeFlagCondition.h"

#include "SigilNarrativeSubsystem.h"

bool USigilNarrativeFlagCondition::Evaluate_Implementation(const FSigilNarrativeContext& Context) const
{
	return Context.NarrativeSubsystem && Context.NarrativeSubsystem->HasFlag(Flag);
}
