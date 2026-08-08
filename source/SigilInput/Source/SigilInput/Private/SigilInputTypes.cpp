// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilInputTypes.h"

#include "SigilInputFunctionLibrary.h"

bool FSigilInputBufferWindow::operator==(const FGameplayTag& OtherTag) const
{
	return Tag == OtherTag;
}

bool FSigilInputBufferWindow::operator!=(const FGameplayTag& OtherTag) const
{
	return Tag != OtherTag;
}

FString FSigilBufferedInput::ToString() const
{
	return FString::Format(TEXT("Tag:{0},Event:{1},Source:{2}"), {
		                       InputTag.IsValid() ? *USigilInputFunctionLibrary::GetLastTagName(InputTag).ToString() : TEXT("None"),
		                       *USigilInputFunctionLibrary::GetTriggerEventString(TriggerEvent),
		                       ActionData.GetSourceAction() ? ActionData.GetSourceAction()->GetName() : TEXT("None"),
	                       });
}
