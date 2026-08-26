// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativePresentation.h"
#include "SigilQuestAsset.h"
#include "SigilStoryAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SigilNarrativeSubsystem.generated.h"

class USigilNarrativeCatalog;

USTRUCT()
struct SIGILNARRATIVE_API FSigilQuestRuntimeState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USigilQuestAsset> QuestAsset = nullptr;

	UPROPERTY()
	ESigilQuestStatus Status = ESigilQuestStatus::NotStarted;

	UPROPERTY()
	FName CurrentStateId;

	UPROPERTY()
	TMap<FName, int32> TaskProgress;

	UPROPERTY()
	bool bCallbackInProgress = false;
};

USTRUCT()
struct SIGILNARRATIVE_API FSigilStoryRuntimeState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USigilStoryAsset> StoryAsset = nullptr;

	UPROPERTY()
	FName ActiveBeatId;

	UPROPERTY()
	TSet<FName> CompletedBeatIds;

	UPROPERTY()
	bool bCallbackInProgress = false;
};

UCLASS()
class SIGILNARRATIVE_API USigilNarrativeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative", meta = (AdvancedDisplay = "bEnabled"))
	void SetFlag(FName Flag, bool bEnabled = true);

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	bool HasFlag(FName Flag) const;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool StartQuest(USigilQuestAsset* QuestAsset, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool AddQuestTaskProgress(FName QuestId, FName TaskId, int32 Delta = 1);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool TryTakeQuestTransition(FName QuestId, FName TransitionId, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	FName GetQuestState(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	ESigilQuestStatus GetQuestStatus(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	int32 GetQuestTaskProgress(FName QuestId, FName TaskId) const;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool EnterStoryBeat(USigilStoryAsset* StoryAsset, FName BeatId, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool CompleteStoryBeat(FName StoryId, FName BeatId, UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	FName GetActiveStoryBeat(FName StoryId) const;

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	bool IsStoryBeatCompleted(FName StoryId, FName BeatId) const;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative|Presentation", meta = (MustImplement = "/Script/SigilNarrative.SigilNarrativePresentationHost"))
	bool RegisterPresentationHost(UObject* InHost);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative|Presentation")
	bool UnregisterPresentationHost(UObject* ExpectedHost);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative|Presentation")
	FSigilNarrativePresentationHandle BeginStoryPresentation(
		FName StoryId,
		FName BeatId,
		UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative|Presentation")
	bool ResolveStoryPresentation(
		FSigilNarrativePresentationHandle Handle,
		ESigilNarrativePresentationResult Result,
		UObject* ContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative|Presentation")
	bool CancelStoryPresentation(FSigilNarrativePresentationHandle Handle);

	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative|Presentation")
	bool HasActivePresentation() const;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool ExportSnapshotJson(FString& OutJson) const;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool ImportSnapshotJson(const FString& JsonText, USigilNarrativeCatalog* Catalog);

private:
	friend class USigilDialogueSession;

	void BeginDialogueCallbackDispatch();
	void EndDialogueCallbackDispatch();
	bool EnterQuestState(FName QuestId, FName StateId, UObject* ContextObject);
	bool IsQuestRuntimeExpected(FName QuestId, const USigilQuestAsset* QuestAsset, ESigilQuestStatus Status, FName StateId) const;
	void EndQuestCallbackDispatch(FName QuestId, const USigilQuestAsset* QuestAsset);
	bool IsStoryRuntimeExpected(FName StoryId, const USigilStoryAsset* StoryAsset, FName ActiveBeatId) const;
	void EndStoryCallbackDispatch(FName StoryId, const USigilStoryAsset* StoryAsset);
	void CancelStoryEnterDispatch(FName StoryId, const USigilStoryAsset* StoryAsset, bool bRemovePendingRuntime);

	UPROPERTY()
	TSet<FName> Flags;

	UPROPERTY()
	TMap<FName, FSigilQuestRuntimeState> QuestStates;

	UPROPERTY()
	TMap<FName, FSigilStoryRuntimeState> StoryStates;

	UPROPERTY(Transient)
	TWeakObjectPtr<UObject> PresentationHost;

	UPROPERTY(Transient)
	FSigilNarrativePresentationHandle ActivePresentationHandle;

	UPROPERTY(Transient)
	FName ActivePresentationStoryId;

	UPROPERTY(Transient)
	FName ActivePresentationBeatId;

	int32 NextPresentationGeneration = 0;

	int32 ActiveDialogueCallbackCount = 0;
};
