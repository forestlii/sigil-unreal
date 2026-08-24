// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNarrativeCondition.h"

bool USigilNarrativeCondition::IsMet(const FSigilNarrativeContext& Context) const
{
	return Evaluate(Context) != bNegate;
}

bool USigilNarrativeCondition::Evaluate_Implementation(const FSigilNarrativeContext& Context) const
{
	return true;
}
