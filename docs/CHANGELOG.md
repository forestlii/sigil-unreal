[English](CHANGELOG.md) | [简体中文](CHANGELOG.zh-CN.md)

# Changelog

Sigil is pre-1.0: minor versions may contain breaking changes. Each entry lists public API changes with the migration path.

## 0.1.x (unreleased) — review hardening

### Breaking changes

| Change | Migration |
|---|---|
| **sigil.combat** — `USigilAttackResultProcessor_Death` removed (it had no behaviour). | Subclass `USigilAttackResultProcessor` (see `_GameplayEvent` / `_GameplayCue`) and implement your death contract. |
| **sigil.combat** — `ServerPlayPredictableMontageForTarget` is no longer `BlueprintCallable`. | Call `PlayPredictableMontageForTarget`; it assigns the request id, predicts locally and sends the RPC. |
| **sigil.combat** — Predictable montage default authorization now rejects any target other than the instigator itself. | Set `MaxPredictableMontageTargetDistance > 0` in Project Settings (same-world targets within that distance) or override `CanPlayMontageOnTarget`. |
| **sigil.combat** — Predictable montage requests must be linear montages (no looping / non-linear section graph); play rate must be within `Min/MaxPredictableMontagePlayRate` (defaults 0.1–4.0). | Split looping montages into linear assets, or drive them outside the predictable path. |
| **sigil.combat** — `FSigilPlayMontageRequest` gained `RequestId` (read-only, auto-assigned); `FSigilReplicatedMontageInfo` gained `StartTimeSeconds`, `RequestId`, `RootTranslationScale`. | Nothing to do unless you serialize these structs yourself. |
| **sigil.combat** — `USigilCombatSystemComponent::PlayPredictedMontage` now returns `bool`. | Ignore the return value if you called it before. |
| **sigil.gas** — `FSigilAttributeGroupName::GetName()` encodes sub-groups as `Main->Sub` (was `Main.Sub`, which the engine mis-parsed). | Rename CurveTable rows from `Main.Sub.Set.Attr` to `Main->Sub.Set.Attr`. |
| **sigil.gas** — `WhenPhaseStartsOrIsActive` / `WhenPhaseEnds` return `FSigilGamePhaseObserverHandle` (was `void`, briefly `FDelegateHandle`); `RemovePhaseObserver` takes that handle and is `BlueprintCallable`; the Blueprint nodes now return the handle too. | Store the returned handle if you need to unregister; otherwise nothing to do. |
| **sigil.gas** — Immediate notification of `WhenPhaseStartsOrIsActive` now honours `MatchType` and passes the actual active phase tag. | `ExactMatch` observers registered while only a child phase is active no longer fire immediately. |
| **sigil.ui** — `USigilGameUIExtensionPointWidget::CheckPlayerState()` removed (duplicate of the retry path). | Nothing to call; registration retries automatically. |
| **sigil.ui** — `USigilGameModalWidget::SetupModal` returns `bool`; a failed setup closes the modal with `Unknown` on activation. | Nothing to do for callers using `USigilAsyncAction_ShowModel`. |
| **sigil.movement** — `FSigilJumpStateSetting::bIsShowDebug` removed. | Delete any Blueprint reads of the field. |
| **sigil.movement** — `bDynamicPlayRate = false` now really disables dynamic play rate (fixed 1.0). | Re-check cycle data that had the flag off. |
| **sigil.movement** — `USigilUtility::CalculateAnimatedSpeed` no longer logs; returns 0 for unusable input. | Cache the result per asset (the default locomotion layer does). |

### Fixes

- **GAS-01 — sigil.gas:** `USigilAbilityCost::CheckCost` now supplies a call-local empty tag container when optional relevant tags are absent, so the Blueprint cost event is not given a null dereference.
- **CBT-01 — sigil.combat:** `AddEntry` marks server storage consumed and dirty before its Flow callback, while client `ConsumeEntry` first marks its local stored entry consumed; callbacks receive an unconsumed value copy, and deferred or reentrant consumption dispatches each entry exactly once.
- **INVC-01 — sigil.inventory:** deserializing a save with a missing item now warns with the stack and item identifiers, skips that stack, and continues valid stacks without inserting a null map entry.
- **INVC-02 — sigil.inventory:** the loadout Server RPC calls the existing local `LoadDefaultLoadouts()` implementation once instead of recursively dispatching through its RPC wrapper.
- **INVG-01 — sigil.inventory:** under stable collection-restriction contracts, pickup preflights both sides, removes from the source first, measures only the picked logical item's source and destination deltas, restores destination rejection only to the source, and counts only a conserved non-zero transfer as success; target-side removal is no longer part of the normal path.
- **INVG-02 — sigil.inventory:** crafting returns `true` only after every requested ingredient removal succeeds; the existing first-failure stop and non-transactional semantics remain unchanged.
- **INVG-03 — sigil.inventory:** random drops build cumulative weights safely, return empty for empty or non-positive totals, choose amounts in the inclusive configured range, and keep final-boundary selection in range.
- **CAM-01 — sigil.camera:** an active empty camera stack returns no view; its component leaves the existing Camera/SpringArm state and pending FOV offset untouched until a valid mode evaluates.

### Automated coverage

- This batch added 19 Automation tests: `SigilGas.AbilityCost` (1), `SigilCombat.AttackResult` (3), `SigilInventory` (12: 2 serialization/loadout, 5 partial-pickup, 2 crafting, and 3 random-drop cases), and `SigilCamera.Stack` (3). The final `SigilInventory` filter executed 14 tests because it also includes 2 existing pickup regressions.
- Fresh final verification passed the `HostEditor Win64 Development` build, all 51 tests selected by `Automation RunTests Sigil`, and an isolated `ProjectSpecterEditor Win64 Development` build using ProjectSpecter `37b1a774` with Sigil `59f3375`. These are compile and Automation results, not runtime gameplay validation.

### Known gaps

- INVC-11 multi-collection deserialization routing was not changed by this batch.
- INVC-12 multi-stack `AddInternal` return-value semantics were not changed; the pickup path now measures the destination's same-logical-item quantity directly instead of relying on that return amount.
- No cross-collection transaction or locking was introduced. Arbitrary Blueprint reentrancy that mutates the same logical item, or a source restriction that dynamically rejects restoration after preflight, remains non-atomic and is not guaranteed by INVG-01.
- PIE, multiplayer runtime, Win64 Cook, and packaged runtime validation were not executed by this batch.
