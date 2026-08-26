// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNarrativePresentation.h"

bool ISigilNarrativePresentationHost::CanBeginPresentation_Implementation(
	USigilNarrativePresentationAsset* Presentation,
	const FSigilNarrativePresentationHandle Handle,
	UObject* ContextObject) const
{
	return false;
}

bool ISigilNarrativePresentationHost::BeginPresentation_Implementation(
	USigilNarrativePresentationAsset* Presentation,
	const FSigilNarrativePresentationHandle Handle,
	UObject* ContextObject)
{
	return false;
}

void ISigilNarrativePresentationHost::CancelPresentation_Implementation(
	const FSigilNarrativePresentationHandle Handle)
{
}
