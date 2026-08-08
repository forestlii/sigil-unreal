// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Interaction/Behaviors/GGS_GameplayBehaviorConfig_InteractionWithAbility.h"
#include "Interaction/Behaviors/GGS_GameplayBehavior_InteractionWithAbility.h"

UGGS_GameplayBehaviorConfig_InteractionWithAbility::UGGS_GameplayBehaviorConfig_InteractionWithAbility()
{
	BehaviorClass = UGGS_GameplayBehavior_InteractionWithAbility::StaticClass();
}

#if WITH_EDITORONLY_DATA
EDataValidationResult UGGS_GameplayBehaviorConfig_InteractionWithAbility::IsDataValid(class FDataValidationContext& Context) const
{
	if (BehaviorClass == nullptr || !BehaviorClass->GetClass()->IsChildOf(UGGS_GameplayBehavior_InteractionWithAbility::StaticClass()))
	{
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
