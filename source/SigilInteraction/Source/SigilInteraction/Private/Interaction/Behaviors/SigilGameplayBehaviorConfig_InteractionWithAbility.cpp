// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Interaction/Behaviors/SigilGameplayBehaviorConfig_InteractionWithAbility.h"
#include "Interaction/Behaviors/SigilGameplayBehavior_InteractionWithAbility.h"

USigilGameplayBehaviorConfig_InteractionWithAbility::USigilGameplayBehaviorConfig_InteractionWithAbility()
{
	BehaviorClass = USigilGameplayBehavior_InteractionWithAbility::StaticClass();
}

#if WITH_EDITORONLY_DATA
EDataValidationResult USigilGameplayBehaviorConfig_InteractionWithAbility::IsDataValid(class FDataValidationContext& Context) const
{
	if (BehaviorClass == nullptr || !BehaviorClass->GetClass()->IsChildOf(USigilGameplayBehavior_InteractionWithAbility::StaticClass()))
	{
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}
#endif
