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

See the commit history for the two review batches of 2026-08-17 (`fix(...)` commits): montage RPC validation / rate limit / rollback protocol, AttackResult FastArray dirty-marking and re-entrancy, timer lifetimes, GamePhase null-guards, camera blend-info / FOV offset / penetration avoidance, modal failure lifecycle, movement play-rate safety and thread-safety, plus two non-editor build fixes.

### Known gaps

- No automated tests yet (planned: FastArray, montage protocol, attribute groups, UI timing, movement speed, camera stack).
- Runtime / networked behaviour has been reasoned about and compiled, not exercised in PIE or multi-client sessions.
