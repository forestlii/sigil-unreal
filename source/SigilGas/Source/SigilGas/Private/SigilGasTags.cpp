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
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SourceObjectInactive, "Sigil.Ability.ActivateFail.SourceObjectInactive", "Ability failed to activate because its SourceObject is missing or reports inactive through ISigilAbilitySourceInterface.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Missing_AbilityAction, "Sigil.Ability.ActivateFail.Missing.AbilityAction", "Ability failed to activate because no ability action matched the request.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Missing_AttackInstigator, "Sigil.Ability.ActivateFail.Missing.AttackInstigator", "Ability failed to activate because the attack instigator could not be resolved.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Missing_MovementInput, "Sigil.Ability.ActivateFail.Missing.MovementInput", "Ability failed to activate because it requires movement input and none was present.");
}

namespace SigilAbilityTraitTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActivationOnSpawn, "Sigil.Ability.Trait.ActivationOnSpawn", "Abilities with this tag will be activated right after granted.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Persistent, "Sigil.Ability.Trait.Persistent", "Abilities with this tag should be persistent during gameplay.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MovementCancellable, "Sigil.Ability.Trait.MovementCancellable", "Abilities with this tag can be cancelled by movement input.");

}

namespace SigilStateTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interacting, "Sigil.State.Interacting", "Owner is currently interacting.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InteractingRemoval, "Sigil.State.InteractingRemoval", "Pending removals of the interacting state; compared against Sigil.State.Interacting counts.");
}

namespace SigilSetByCallerTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CooldownDuration, "Sigil.SetByCaller.CooldownDuration", "SetByCaller data tag for the duration of a shared cooldown GameplayEffect; written by USigilGameplayAbility::ApplyCooldown from CooldownDuration.");
}

namespace SigilCooldownTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SharedMarker, "Sigil.Cooldown.Shared", "Granted by shared cooldown GameplayEffects so they pass IsDataValid; excluded from cooldown matching by USigilGameplayAbility::GetCooldownTags when CooldownTags is set.");
}


