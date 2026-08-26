// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilNarrativePresentationTestTypes.h"

bool USigilNarrativePresentationTestHost::CanBeginPresentation_Implementation(
	USigilNarrativePresentationAsset* Presentation,
	const FSigilNarrativePresentationHandle Handle,
	UObject* ContextObject) const
{
	return bCanBegin && Presentation && Handle.IsValid();
}

bool USigilNarrativePresentationTestHost::BeginPresentation_Implementation(
	USigilNarrativePresentationAsset* Presentation,
	const FSigilNarrativePresentationHandle Handle,
	UObject* ContextObject)
{
	++BeginCount;
	LastHandle = Handle;
	return bAcceptBegin && Presentation && Handle.IsValid();
}

void USigilNarrativePresentationTestHost::CancelPresentation_Implementation(
	const FSigilNarrativePresentationHandle Handle)
{
	++CancelCount;
	LastHandle = Handle;
}
