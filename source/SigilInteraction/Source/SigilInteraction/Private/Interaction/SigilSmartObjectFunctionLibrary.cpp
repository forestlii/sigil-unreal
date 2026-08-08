// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/SigilSmartObjectFunctionLibrary.h"
#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "GameplayBehaviorConfig.h"
#include "SmartObjectBlueprintFunctionLibrary.h"
#include "SmartObjectDefinition.h"
#include "Engine/World.h"
#include "SmartObjectSubsystem.h"
#include "Interaction/SigilInteractionDefinition.h"

UGameplayBehaviorConfig* USigilSmartObjectFunctionLibrary::GetGameplayBehaviorConfig(const USmartObjectBehaviorDefinition* BehaviorDefinition)
{
	if (const UGameplayBehaviorSmartObjectBehaviorDefinition* Definition = Cast<UGameplayBehaviorSmartObjectBehaviorDefinition>(BehaviorDefinition))
	{
		return Definition->GameplayBehaviorConfig;
	}

	return nullptr;
}

bool USigilSmartObjectFunctionLibrary::FindGameplayBehaviorConfig(const USmartObjectBehaviorDefinition* BehaviorDefinition, TSubclassOf<UGameplayBehaviorConfig> DesiredClass,
                                                                 UGameplayBehaviorConfig*& OutConfig)
{
	if (UClass* RealClass = DesiredClass)
	{
		if (UGameplayBehaviorConfig* Config = GetGameplayBehaviorConfig(BehaviorDefinition))
		{
			if (Config->GetClass()->IsChildOf(RealClass))
			{
				OutConfig = Config;
				return true;
			}
		}
	}
	return false;
}

bool USigilSmartObjectFunctionLibrary::FindSmartObjectsWithInteractionEntranceInActor(const FSmartObjectRequestFilter& Filter, AActor* SearchActor, TArray<FSmartObjectRequestResult>& OutResults,
                                                                                     const AActor* UserActor)
{
	if (!IsValid(SearchActor))
	{
		return false;
	}
	TArray<FSmartObjectRequestResult> Results;
	USmartObjectBlueprintFunctionLibrary::FindSmartObjectsInActor(Filter, SearchActor, Results, UserActor);
	if (Results.IsEmpty())
	{
		return false;
	}

	// filter results which has definiton entry.
	for (int32 i = 0; i < Results.Num(); i++)
	{
		USigilInteractionDefinition* FoundDefinition;
		if (FindInteractionDefinitionFromSmartObjectSlot(SearchActor, Results[i].SlotHandle, FoundDefinition))
		{
			OutResults.Add(Results[i]);
		}
	}
	return !OutResults.IsEmpty();
}

bool USigilSmartObjectFunctionLibrary::FindInteractionDefinitionFromSmartObjectSlot(UObject* WorldContext, FSmartObjectSlotHandle SmartObjectSlotHandle, USigilInteractionDefinition*& OutDefinition)
{
	if (WorldContext && WorldContext->GetWorld() && SmartObjectSlotHandle.IsValid())
	{
		if (USmartObjectSubsystem* Subsystem = WorldContext->GetWorld()->GetSubsystem<USmartObjectSubsystem>())
		{
			USigilInteractionDefinition* FoundDefinition = nullptr;
			Subsystem->ReadSlotData(SmartObjectSlotHandle, [&FoundDefinition](FConstSmartObjectSlotView SlotView)
			{
				if (const FSigilSmartObjectInteractionEntranceData* Entry = SlotView.GetDefinitionDataPtr<FSigilSmartObjectInteractionEntranceData>())
				{
					if (!Entry->DefinitionDA.IsNull())
					{
						FoundDefinition = Entry->DefinitionDA.LoadSynchronous();
					}
				}
			});

			if (FoundDefinition)
			{
				OutDefinition = FoundDefinition;
				return true;
			}
		}
	}
	return false;
}
