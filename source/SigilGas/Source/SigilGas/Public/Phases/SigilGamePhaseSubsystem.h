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

	//TODO Return a handle so folks can delete these.  They will just grow until the world resets.
	//TODO Should we just occasionally clean these observers up?  It's not as if everyone will properly unhook them even if there is a handle.
	void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseActive);
	void WhenPhaseEnds(FGameplayTag PhaseTag, ESigilPhaseTagMatchType MatchType, const FGGamePhaseTagDelegate& WhenPhaseEnd);

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

	friend class USigilGamePhaseAbility;
};
