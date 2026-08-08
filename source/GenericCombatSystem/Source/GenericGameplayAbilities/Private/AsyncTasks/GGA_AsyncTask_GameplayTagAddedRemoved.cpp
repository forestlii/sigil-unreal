// Copyright 2025 RedMoonGames All Rights Reserved.


#include "AsyncTasks/GGA_AsyncTask_GameplayTagAddedRemoved.h"

UGGA_AsyncTask_GameplayTagAddedRemoved* UGGA_AsyncTask_GameplayTagAddedRemoved::ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer InTags)
{
	UGGA_AsyncTask_GameplayTagAddedRemoved* TaskInstance = NewObject<UGGA_AsyncTask_GameplayTagAddedRemoved>();
	TaskInstance->SetAbilitySystemComponent(AbilitySystemComponent);
	TaskInstance->Tags = InTags;

	if (!IsValid(AbilitySystemComponent) || InTags.Num() < 1)
	{
		TaskInstance->EndTask();
		return nullptr;
	}

	TArray<FGameplayTag> TagArray;
	InTags.GetGameplayTagArray(TagArray);

	for (FGameplayTag Tag : TagArray)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(TaskInstance, &UGGA_AsyncTask_GameplayTagAddedRemoved::TagChanged);
	}

	return TaskInstance;
}

void UGGA_AsyncTask_GameplayTagAddedRemoved::EndTask()
{
	EndAction();
}

void UGGA_AsyncTask_GameplayTagAddedRemoved::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		TArray<FGameplayTag> TagArray;
		Tags.GetGameplayTagArray(TagArray);

		for (FGameplayTag Tag : TagArray)
		{
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	Super::EndAction();
}

void UGGA_AsyncTask_GameplayTagAddedRemoved::TagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		OnTagAdded.Broadcast(Tag);
	}
	else
	{
		OnTagRemoved.Broadcast(Tag);
	}
}
