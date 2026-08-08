// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Interaction/GGS_SmartObjectFunctionLibrary.h"
#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "GameplayBehaviorConfig.h"
#include "SmartObjectBlueprintFunctionLibrary.h"
#include "SmartObjectDefinition.h"
#include "Engine/World.h"
#include "SmartObjectSubsystem.h"
#include "Interaction/GGS_InteractionDefinition.h"

UGameplayBehaviorConfig* UGGS_SmartObjectFunctionLibrary::GetGameplayBehaviorConfig(const USmartObjectBehaviorDefinition* BehaviorDefinition)
{
	if (const UGameplayBehaviorSmartObjectBehaviorDefinition* Definition = Cast<UGameplayBehaviorSmartObjectBehaviorDefinition>(BehaviorDefinition))
	{
		return Definition->GameplayBehaviorConfig;
	}

	return nullptr;
}

bool UGGS_SmartObjectFunctionLibrary::FindGameplayBehaviorConfig(const USmartObjectBehaviorDefinition* BehaviorDefinition, TSubclassOf<UGameplayBehaviorConfig> DesiredClass,
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

bool UGGS_SmartObjectFunctionLibrary::FindSmartObjectsWithInteractionEntranceInActor(const FSmartObjectRequestFilter& Filter, AActor* SearchActor, TArray<FSmartObjectRequestResult>& OutResults,
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
		UGGS_InteractionDefinition* FoundDefinition;
		if (FindInteractionDefinitionFromSmartObjectSlot(SearchActor, Results[i].SlotHandle, FoundDefinition))
		{
			OutResults.Add(Results[i]);
		}
	}
	return !OutResults.IsEmpty();
}

bool UGGS_SmartObjectFunctionLibrary::FindInteractionDefinitionFromSmartObjectSlot(UObject* WorldContext, FSmartObjectSlotHandle SmartObjectSlotHandle, UGGS_InteractionDefinition*& OutDefinition)
{
	if (WorldContext && WorldContext->GetWorld() && SmartObjectSlotHandle.IsValid())
	{
		if (USmartObjectSubsystem* Subsystem = WorldContext->GetWorld()->GetSubsystem<USmartObjectSubsystem>())
		{
			UGGS_InteractionDefinition* FoundDefinition = nullptr;
			Subsystem->ReadSlotData(SmartObjectSlotHandle, [&FoundDefinition](FConstSmartObjectSlotView SlotView)
			{
				if (const FGGS_SmartObjectInteractionEntranceData* Entry = SlotView.GetDefinitionDataPtr<FGGS_SmartObjectInteractionEntranceData>())
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
