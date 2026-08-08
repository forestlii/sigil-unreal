// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "AsyncTasks/SigilAsyncTask_GameplayTagAddedRemoved.h"

USigilAsyncTask_GameplayTagAddedRemoved* USigilAsyncTask_GameplayTagAddedRemoved::ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer InTags)
{
	USigilAsyncTask_GameplayTagAddedRemoved* TaskInstance = NewObject<USigilAsyncTask_GameplayTagAddedRemoved>();
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
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(TaskInstance, &USigilAsyncTask_GameplayTagAddedRemoved::TagChanged);
	}

	return TaskInstance;
}

void USigilAsyncTask_GameplayTagAddedRemoved::EndTask()
{
	EndAction();
}

void USigilAsyncTask_GameplayTagAddedRemoved::EndAction()
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

void USigilAsyncTask_GameplayTagAddedRemoved::TagChanged(const FGameplayTag Tag, int32 NewCount)
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
