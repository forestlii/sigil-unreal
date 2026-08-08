// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIPS_InputFunctionLibrary.h"

#include "GameplayTagsManager.h"

FInputActionValue UGIPS_InputFunctionLibrary::GetInputActionValue(const FInputActionInstance& ActionDataData)
{
	return ActionDataData.GetValue();
}

FName UGIPS_InputFunctionLibrary::GetLastTagName(FGameplayTag Tag)
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

FString UGIPS_InputFunctionLibrary::GetSimpleStringOfTags(FGameplayTagContainer Tags)
{
	return Tags.ToStringSimple();
}

TArray<FName> UGIPS_InputFunctionLibrary::GetLastTagNameArray(FGameplayTagContainer Tags)
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

FString UGIPS_InputFunctionLibrary::GetLastTagNameString(FGameplayTagContainer Tags)
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

FString UGIPS_InputFunctionLibrary::GetTagQueryDescription(const FGameplayTagQuery& TagQuery)
{
	if (TagQuery.IsEmpty())
	{
		return TEXT("Empty Query");
	}

	return TagQuery.GetDescription();
}

FString UGIPS_InputFunctionLibrary::GetTriggerEventString(ETriggerEvent TriggerEvent)
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
