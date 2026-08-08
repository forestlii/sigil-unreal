// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Targeting/Filters/GCS_TargetingFilterTask_TagsRequirements.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

bool UGCS_TargetingFilterTask_TagsRequirements::ShouldFilterTarget(const FTargetingRequestHandle& TargetingHandle, const FTargetingDefaultResultData& TargetData) const
{
	const AActor* TargetActor = TargetData.HitResult.GetActor();

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
	{
		FGameplayTagContainer ActorTags;
		ASC->GetOwnedGameplayTags(ActorTags);

		return bInvert ? !TagQuery.Matches(ActorTags) : TagQuery.Matches(ActorTags);
	}

	if (bLookingForTagAssetInterface)
	{
		const IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(TargetActor);
		if (!TagAssetInterface)
		{
			const TArray<UActorComponent*> Components = TargetActor->GetComponentsByInterface(UGameplayTagAssetInterface::StaticClass());
			TagAssetInterface = Components.IsValidIndex(0) ? Cast<IGameplayTagAssetInterface>(Components[0]) : nullptr;
		}

		if (TagAssetInterface)
		{
			FGameplayTagContainer ActorTags;
			TagAssetInterface->GetOwnedGameplayTags(ActorTags);

			return bInvert ? !TagQuery.Matches(ActorTags) : TagQuery.Matches(ActorTags);
		}
	}

	return false;
}
