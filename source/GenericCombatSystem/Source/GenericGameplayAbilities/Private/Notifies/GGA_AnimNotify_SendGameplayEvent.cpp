// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Notifies/GGA_AnimNotify_SendGameplayEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

void UGGA_AnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

FString UGGA_AnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	
	FString NotifyName = FString::Format(TEXT("SendEvent:{0}"),{GetLastTagName(EventTag).ToString()});
	return NotifyName;
}
