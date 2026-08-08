// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"


namespace SigilAbilityActivateFailTags
{
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cost)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagsBlocked);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagsMissing);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Networking);
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivationGroup);
}

namespace SigilAbilityTraitTags
{
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivationOnSpawn)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Persistent)

}

namespace SigilStateTags
{
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interacting)
	SIGILGAS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InteractingRemoval)
}
