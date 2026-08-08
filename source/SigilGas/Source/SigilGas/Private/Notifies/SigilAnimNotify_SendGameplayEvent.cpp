// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Notifies/SigilAnimNotify_SendGameplayEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

void USigilAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (EventTag.IsValid() && MeshComp->GetOwner())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
		if (ASC == nullptr)
			return;
		FGameplayEventData EventData;
		EventData.Instigator = MeshComp->GetOwner();
		EventData.EventMagnitude = EventReference.GetNotify()->GetTime();
		EventData.EventTag = EventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(),EventTag,EventData);
	}
}

FName GetLastTagName(FGameplayTag Tag)
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

FString USigilAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	
	FString NotifyName = FString::Format(TEXT("SendEvent:{0}"),{GetLastTagName(EventTag).ToString()});
	return NotifyName;
}
