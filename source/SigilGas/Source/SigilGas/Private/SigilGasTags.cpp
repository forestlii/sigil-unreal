// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilGasTags.h"

namespace SigilAbilityActivateFailTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown, "Sigil.Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cost, "Sigil.Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TagsBlocked, "Sigil.Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TagsMissing, "Sigil.Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Networking, "Sigil.Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivationGroup, "Sigil.Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");
}

namespace SigilAbilityTraitTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivationOnSpawn, "Sigil.Ability.Trait.ActivationOnSpawn", "Abilities with this tag will be activated right after granted.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Persistent, "Sigil.Ability.Trait.Persistent", "Abilities with this tag should be persistent during gameplay.");

}

namespace SigilStateTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interacting, "Sigil.State.Interacting", "Owner is currently interacting.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InteractingRemoval, "Sigil.State.InteractingRemoval", "Pending removals of the interacting state; compared against Sigil.State.Interacting counts.");
}


