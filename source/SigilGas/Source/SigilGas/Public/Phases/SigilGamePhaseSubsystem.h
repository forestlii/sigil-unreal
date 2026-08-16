// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "SigilGamePhaseSubsystem.generated.h"

class USigilGamePhaseAbility;

DECLARE_DYNAMIC_DELEGATE_OneParam(FGGamePhaseDynamicDelegate, const USigilGamePhaseAbility*, Phase);

DECLARE_DELEGATE_OneParam(FGGamePhaseDelegate, const USigilGamePhaseAbility* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FGGamePhaseTagDynamicDelegate, const FGameplayTag&, PhaseTag);

DECLARE_DELEGATE_OneParam(FGGamePhaseTagDelegate, const FGameplayTag& PhaseTag);

// Match rule for message receivers
UENUM(BlueprintType)
enum class ESigilPhaseTagMatchType : uint8
{
	// An exact match will only receive messages with exactly the same channel
	// (e.g., registering for "A.B" will match a broadcast of A.B but not A.B.C)
	ExactMatch,

	// A partial match will receive any messages rooted in the same channel
	// (e.g., registering for "A.B" will match a broadcast of A.B as well as A.B.C)
	PartialMatch
};

/**
 * Handle returned when registering a game-phase observer; pass it to RemovePhaseObserver to unregister.
 * Every registration gets a unique id, even when the same delegate is registered twice.
 * 注册游戏阶段观察者时返回的句柄；传给 RemovePhaseObserver 以注销。每次注册都有唯一 id，同一委托注册两次也各自独立。
 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilGamePhaseObserverHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Id{0};

	bool IsValid() const { return Id != 0; }
};


/** Subsystem for managing game phases using gameplay tags in a nested manner, which allows parent and child 
 * phases to be active at the same time, but not sibling phases.
 * Example:  Game.Playing and Game.Playing.WarmUp can coexist, but Game.Playing and Game.ShowingScore cannot. 
 * When a new phase is started, any active phases that are not ancestors will be ended.
 * Example: if Game.Playing and Game.Playing.CaptureTheFlag are active when Game.Playing.PostGame is started, 
 *     Game.Playing will remain active, while Game.Playing.CaptureTheFlag will end.
 */
UCLASS()
class SIGILGAS_API USigilGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USigilGamePhaseSubsystem();

	//virtual void PostInitialize() override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	void StartPhase(TSubclassOf<USigilGamePhaseAbility> PhaseAbility, FGGamePhaseDelegate PhaseEndedCallback = FGGamePhaseDelegate());

	/**
	 * Registers an observer for phase start. If a matching phase is already active the callback fires immediately
	 * with that phase's actual tag (respecting MatchType). Observers are bound weakly through their delegate: once
	 * the bound object is destroyed the entry is pruned automatically on the next phase transition, so listeners
	 * that never unhook cannot grow the list unbounded. Returns a unique handle for explicit removal.
	 * 注册阶段开始观察者。若已有匹配的阶段处于活动状态，回调会立即以该阶段的实际 Tag 触发（遵循 MatchType）。
	 * 观察者通过委托弱绑定：绑定对象销毁后，条目会在下一次阶段切换时自动清理，因而从不解绑的监听者不会让列表
	 * 无限增长。返回唯一句柄以便显式移除。
	 */
	FSigilGamePhaseObserverHandle WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseActive);
	FSigilGamePhaseObserverHandle WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseEnd);

	/** Removes an observer previously returned by WhenPhaseStartsOrIsActive / WhenPhaseEnds (C++ or Blueprint). 移除上述函数返回句柄对应的观察者（C++ 或蓝图）。 */
	UFUNCTION(BlueprintCallable, Category = "GGA|GamePhase")
	void RemovePhaseObserver(FSigilGamePhaseObserverHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "GGA|GamePhase", BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|GamePhase", meta = (DisplayName="Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<USigilGamePhaseAbility> Phase, const FGGamePhaseDynamicDelegate& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|GamePhase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	FSigilGamePhaseObserverHandle K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, FGGamePhaseTagDynamicDelegate WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|GamePhase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	FSigilGamePhaseObserverHandle K2_WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, FGGamePhaseTagDynamicDelegate WhenPhaseEnd);

	void OnBeginPhase(const USigilGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	void OnEndPhase(const USigilGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);

	/** Resolves the GameState's ASC, logging (not asserting) when the GameState or ASC is not available yet. */
	UAbilitySystemComponent* GetGameStateAbilitySystem(const TCHAR* Context) const;

private:
	struct FGGamePhaseEntry
	{
	public:
		FGameplayTag PhaseTag;
		FGGamePhaseDelegate PhaseEndedCallback;
	};

	TMap<FGameplayAbilitySpecHandle, FGGamePhaseEntry> ActivePhaseMap;

	struct FGPhaseObserver
	{
	public:
		bool IsMatch(const FGameplayTag& ComparePhaseTag) const;

		int32 Id = 0;
		FGameplayTag PhaseTag;
		ESigilPhaseTagMatchType MatchType = ESigilPhaseTagMatchType::ExactMatch;
		FGGamePhaseTagDelegate PhaseCallback;
	};

	TArray<FGPhaseObserver> PhaseStartObservers;
	TArray<FGPhaseObserver> PhaseEndObservers;
	int32 NextObserverId = 1;

	/** Prunes dead observers, then invokes every observer matching PhaseTag (over a snapshot, so callbacks may register new observers). */
	static void NotifyObservers(TArray<FGPhaseObserver>& Observers, const FGameplayTag& PhaseTag);

	friend class USigilGamePhaseAbility;
};
