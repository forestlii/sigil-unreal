// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilInputFunctionLibrary.h"

#include "GameplayTagsManager.h"

FInputActionValue USigilInputFunctionLibrary::GetInputActionValue(const FInputActionInstance& ActionDataData)
{
	return ActionDataData.GetValue();
}

FName USigilInputFunctionLibrary::GetLastTagName(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return FName(TEXT("Invalid Tag"));
	}

	TArray<FName> TagNames;

	UGameplayTagsManager::Get().SplitGameplayTagFName(Tag, TagNames);

	if (TagNames.IsEmpty())
	{
		return FName(TEXT("Invalid Tag"));
	}

	return TagNames.Last();
}

FString USigilInputFunctionLibrary::GetSimpleStringOfTags(FGameplayTagContainer Tags)
{
	return Tags.ToStringSimple();
}

TArray<FName> USigilInputFunctionLibrary::GetLastTagNameArray(FGameplayTagContainer Tags)
{
	TArray<FGameplayTag> TagArray;
	Tags.GetGameplayTagArray(TagArray);

	TArray<FName> NameArray;
	for (const FGameplayTag& Tag : TagArray)
	{
		NameArray.Add(GetLastTagName(Tag));
	}

	return NameArray;
}

FString USigilInputFunctionLibrary::GetLastTagNameString(FGameplayTagContainer Tags)
{
	TArray<FGameplayTag> TagArray;
	Tags.GetGameplayTagArray(TagArray);

	FString Output;
	for (const FGameplayTag& Tag : TagArray)
	{
		Output.Append(FString::Format(TEXT(" ({0}) "), {GetLastTagName(Tag).ToString()}));
	}

	return Output;
}

FString USigilInputFunctionLibrary::GetTagQueryDescription(const FGameplayTagQuery& TagQuery)
{
	if (TagQuery.IsEmpty())
	{
		return TEXT("Empty Query");
	}

	return TagQuery.GetDescription();
}

FString USigilInputFunctionLibrary::GetTriggerEventString(ETriggerEvent TriggerEvent)
{
	switch (TriggerEvent)
	{
	case ETriggerEvent::Started:
		return TEXT("Start");
	case ETriggerEvent::Triggered:
		return TEXT("Triggered");
	case ETriggerEvent::Canceled:
		return TEXT("Canceled");
	case ETriggerEvent::Ongoing:
		return TEXT("Ongoing");
	case ETriggerEvent::Completed:
		return TEXT("Completed");
	case ETriggerEvent::None:
	default:
		return TEXT("None");
	}
}
