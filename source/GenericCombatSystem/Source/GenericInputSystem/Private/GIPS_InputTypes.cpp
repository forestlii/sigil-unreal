// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIPS_InputTypes.h"

#include "GIPS_InputFunctionLibrary.h"

bool FGIPS_InputBufferWindow::operator==(const FGameplayTag& OtherTag) const
{
	return Tag == OtherTag;
}

bool FGIPS_InputBufferWindow::operator!=(const FGameplayTag& OtherTag) const
{
	return Tag != OtherTag;
}

FString FGIPS_BufferedInput::ToString() const
{
	return FString::Format(TEXT("Tag:{0},Event:{1},Source:{2}"), {
		                       InputTag.IsValid() ? *UGIPS_InputFunctionLibrary::GetLastTagName(InputTag).ToString() : TEXT("None"),
		                       *UGIPS_InputFunctionLibrary::GetTriggerEventString(TriggerEvent),
		                       ActionData.GetSourceAction() ? ActionData.GetSourceAction()->GetName() : TEXT("None"),
	                       });
}
