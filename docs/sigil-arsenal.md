[English](sigil-arsenal.md) | [简体中文](sigil-arsenal.zh-CN.md)

# sigil.arsenal

`sigil.arsenal` (plugin `SigilArsenal`) turns an inventory item into a **weapon loadout**: the abilities the weapon grants, the per-weapon montage table its abilities read, and the animation layer linked while the weapon is active. It is the bridge that `sigil.inventory` (zero GAS coupling by design) and `sigil.combat` (no inventory knowledge) deliberately leave to the project.

**Dependencies:** `sigil.gas`, `sigil.inventory`, `sigil.combat`. The three base packages stay independent of each other; only projects that want item-driven weapons load this one.

## Model

```
USigilItemDefinition
 ├─ USigilItemFragment_Equippable      InstanceType = USigilWeaponEquipmentInstance, ActorsToSpawn = weapon actor(s)
 └─ USigilItemFragment_WeaponLoadout   AbilitySet · GrantPolicy · AbilityActionSet · AnimLayerClass
            │ equipped into a slot (USigilEquipmentSystemComponent)
            ▼
USigilWeaponEquipmentInstance  ── grants AbilitySet to the pawn's ASC (SourceObject = this instance)
            │                  ── links AnimLayerClass to the main mesh while active
            │                  ── answers QueryAbilityActions from AbilityActionSet
            ▼
weapon actor (ISigilWeaponInterface)   SourceObject = the equipment instance
```

- **Item → abilities.** `USigilItemFragment_WeaponLoadout::AbilitySet` is a `USigilAbilitySet`. `GrantPolicy` picks when it is granted: `WhileEquipped` (default — granted in any slot; gate activation with `USigilGameplayAbility::bRequireSourceObjectActive` so only the active weapon fires, which makes weapon swaps instant) or `WhileActive` (granted on activation, revoked on deactivation).
- **Ability → montage per weapon.** `AbilityActionSet` is a `USigilAbilityActionSetSettings`: one "fire" ability, one montage per weapon. `USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions` resolves the pawn's active weapon and selects its actions — implement `ISigilCombatInterface::QueryAbilityActions` on your character by forwarding to it.
- **Animation layer.** `AnimLayerClass` is linked (`LinkAnimClassLayers`) to the pawn's main mesh (`USigilCombatSystemSettings::CharacterMeshLookupTag`, then `ACharacter::GetMesh`, overridable with `AnimLayerMeshLookupTag`) when the weapon activates and unlinked when it deactivates or is unequipped, on every machine.
- **Reaching the weapon from an ability.** The granted specs use the equipment instance as SourceObject: `USigilArsenalFunctionLibrary::GetWeaponEquipmentOfAbility` gives you the instance, from which `GetSourceItem()` (item state such as ammo), `GetWeaponActor()` (muzzle / traces) and `GetAbilityActionSet()` are one call away. Spawned weapon actors point back through `ISigilWeaponInterface::GetSourceObject`.
- **Switching weapons** is the inventory's slot-group mechanism: `USigilEquipmentSystemComponent::SetGroupActiveIndex` / `CycleGroupActiveIndex`. Activation and deactivation reach the equipment instance through `OnActiveStateChanged`.

## Quick Start

1. Create the slot tags and a `USigilItemSlotCollectionDefinition` for the equipped collection with a weapon slot group (see the sigil.inventory guide).
2. Create a `USigilAbilitySet` with the weapon's abilities. Abilities that must only fire from the active weapon set `bRequireSourceObjectActive`.
3. Create a `USigilAbilityActionSetSettings` per weapon mapping the shared ability tags (for example `Ability.Fire`) to that weapon's montages.
4. Create the weapon's `USigilItemDefinition`: add **Equippable Settings** (`InstanceType` = `USigilWeaponEquipmentInstance` or a subclass, `ActorsToSpawn` = your weapon actor) and **Weapon Loadout Settings** (ability set, action set, optional anim layer).
5. On the character, implement `ISigilCombatInterface::QueryAbilityActions` by calling `USigilArsenalFunctionLibrary::QueryActiveWeaponAbilityActions`.
6. Initialize the ability system **before** the equipment system (the loadout is granted in `OnEquipmentBeginPlay`; without an initialized ASC it logs a warning and grants nothing).

## Magazine cost

Add `USigilAbilityCost_ItemIntegerAttribute` to the single-shot ability's `AdditionalCosts`, and call `CommitAbility` before producing the shot. `Tag` defaults to `Sigil.Arsenal.Ammo.Magazine`, `Quantity` is a positive integer (default 1), and `FailureTag` defaults to `Sigil.Arsenal.Ability.Fail.Ammo`. Projects may replace either tag.

Initialize the magazine with `USigilItemFragment_DynamicAttributes::InitialIntegerAttributes`. Put the capacity on the item definition's `StaticIntegerAttributes` under `Sigil.Arsenal.Ammo.MagazineCapacity`. The cost does not clamp reloads or enforce capacity; reload logic belongs to the consumer.

The cost resolves the weapon equipment from the checked ability spec's `SourceObject`, then its source item. Missing equipment, item, attribute, invalid tag, non-positive quantity, or insufficient ammunition rejects the cost and adds `FailureTag` when configured. Only the equipment's authoritative owning pawn may deduct ammunition; application rechecks availability so direct or repeated application cannot make the count negative. Client checks are read-only and do not predict subtraction.

Reserve ammunition, reload abilities, item AttributeSets and tag-to-attribute mappings are outside this batch. Network reconciliation and live gameplay remain unverified.

## Known gaps

- Magazine cost is implemented; fire cadence is pending. Reserve ammunition and reload logic belong to the consumer.
- The animation layer is linked to a single main mesh; first-person secondary meshes are not covered.
- `USigilEquipmentSystemComponent::SetEquipmentActiveState(slot, false)` on an active entry is a no-op in sigil.inventory; switch through the group index API.
