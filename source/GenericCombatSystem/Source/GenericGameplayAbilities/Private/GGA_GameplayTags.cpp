// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GGA_GameplayTags.h"

namespace GGA_AbilityActivateFailTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown, "GGF.Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cost, "GGF.Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TagsBlocked, "GGF.Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TagsMissing, "GGF.Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Networking, "GGF.Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivationGroup, "GGF.Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");
}

namespace GGA_AbilityTraitTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivationOnSpawn, "GGF.Ability.Trait.ActivationOnSpawn", "Abilities with this tag will be activated right after granted.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Persistent, "GGF.Ability.Trait.Persistent", "Abilities with this tag should be persistent during gameplay.");

}


