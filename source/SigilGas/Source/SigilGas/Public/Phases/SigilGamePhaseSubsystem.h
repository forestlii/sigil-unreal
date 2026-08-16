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
	 * Registers an observer for phase start. Observers are bound weakly through their delegate: once the bound
	 * object is destroyed the entry is pruned automatically on the next phase transition, so listeners that
	 * never unhook cannot grow the list unbounded. Returns the delegate handle for explicit removal.
	 * 注册阶段开始观察者。观察者通过委托弱绑定：绑定对象销毁后，条目会在下一次阶段切换时自动清理，
	 * 因而从不解绑的监听者不会让列表无限增长。返回委托句柄以便显式移除。
	 */
	FDelegateHandle WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseActive);
	FDelegateHandle WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseEnd);

	/** Removes an observer previously returned by WhenPhaseStartsOrIsActive / WhenPhaseEnds. 移除上述函数返回句柄对应的观察者。 */
	void RemovePhaseObserver(FDelegateHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "GGA|GamePhase", BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|GamePhase", meta = (DisplayName="Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<USigilGamePhaseAbility> Phase, const FGGamePhaseDynamicDelegate& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|GamePhase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, FGGamePhaseTagDynamicDelegate WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GGA|GamePhase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, FGGamePhaseTagDynamicDelegate WhenPhaseEnd);

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

		FGameplayTag PhaseTag;
		ESigilPhaseTagMatchType MatchType = ESigilPhaseTagMatchType::ExactMatch;
		FGGamePhaseTagDelegate PhaseCallback;
	};

	TArray<FGPhaseObserver> PhaseStartObservers;
	TArray<FGPhaseObserver> PhaseEndObservers;

	/** Prunes dead observers, then invokes every observer matching PhaseTag (over a snapshot, so callbacks may register new observers). */
	static void NotifyObservers(TArray<FGPhaseObserver>& Observers, const FGameplayTag& PhaseTag);

	friend class USigilGamePhaseAbility;
};
