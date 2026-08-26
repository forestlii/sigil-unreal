// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilStoryEditorModel.h"

#include "ScopedTransaction.h"
#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"

#define LOCTEXT_NAMESPACE "SigilNarrativeEditor"

FStoryBeatStructOnScope::FStoryBeatStructOnScope(
	USigilStoryAsset* InAsset,
	const int32 InBeatIndex)
	: FStructOnScope()
	, Asset(InAsset)
	, BeatIndex(InBeatIndex)
{
}

uint8* FStoryBeatStructOnScope::GetStructMemory()
{
	USigilStoryAsset* StoryAsset = Asset.Get();
	return StoryAsset && StoryAsset->Beats.IsValidIndex(BeatIndex)
		? reinterpret_cast<uint8*>(&StoryAsset->Beats[BeatIndex])
		: nullptr;
}

const uint8* FStoryBeatStructOnScope::GetStructMemory() const
{
	const USigilStoryAsset* StoryAsset = Asset.Get();
	return StoryAsset && StoryAsset->Beats.IsValidIndex(BeatIndex)
		? reinterpret_cast<const uint8*>(&StoryAsset->Beats[BeatIndex])
		: nullptr;
}

const UScriptStruct* FStoryBeatStructOnScope::GetStruct() const
{
	return FSigilStoryBeatDefinition::StaticStruct();
}

UPackage* FStoryBeatStructOnScope::GetPackage() const
{
	return Asset.IsValid() ? Asset->GetOutermost() : nullptr;
}

void FStoryBeatStructOnScope::SetPackage(UPackage* InPackage)
{
	(void)InPackage;
}

bool FStoryBeatStructOnScope::IsValid() const
{
	return GetStructMemory() != nullptr;
}

FSigilStoryEditorModel::FSigilStoryEditorModel(USigilStoryAsset* InAsset)
	: Asset(InAsset)
{
}

bool FSigilStoryEditorModel::SetStoryId(const FName NewStoryId, FText& OutError)
{
	OutError = FText::GetEmpty();
	USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset)
	{
		OutError = LOCTEXT("StoryAssetMissing", "The Story asset is no longer available.");
		return false;
	}
	if (NewStoryId.IsNone())
	{
		OutError = LOCTEXT("EmptyStoryId", "StoryId must not be empty.");
		return false;
	}
	if (StoryAsset->StoryId == NewStoryId)
	{
		return true;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetStoryId", "Set Story ID"));
	StoryAsset->Modify();
	StoryAsset->StoryId = NewStoryId;
	FinishStructuralChange();
	return true;
}

TArray<FName> FSigilStoryEditorModel::GetFilteredBeatIds(const FString& Filter) const
{
	TArray<FName> Result;
	const USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset)
	{
		return Result;
	}

	for (const FSigilStoryBeatDefinition& Beat : StoryAsset->Beats)
	{
		if (Filter.IsEmpty() || Beat.BeatId.ToString().Contains(Filter, ESearchCase::IgnoreCase))
		{
			Result.Add(Beat.BeatId);
		}
	}
	return Result;
}

int32 FSigilStoryEditorModel::FindBeatIndex(const FName BeatId) const
{
	const USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset)
	{
		return INDEX_NONE;
	}

	return StoryAsset->Beats.IndexOfByPredicate([BeatId](const FSigilStoryBeatDefinition& Beat)
	{
		return Beat.BeatId == BeatId;
	});
}

FName FSigilStoryEditorModel::AddBeat()
{
	USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset)
	{
		return NAME_None;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddStoryBeat", "Add Story Beat"));
	StoryAsset->Modify();
	FSigilStoryBeatDefinition& AddedBeat = StoryAsset->Beats.AddDefaulted_GetRef();
	AddedBeat.BeatId = MakeUniqueBeatId(TEXT("Beat"), true);
	const FName AddedBeatId = AddedBeat.BeatId;
	FinishStructuralChange();
	return AddedBeatId;
}

FName FSigilStoryEditorModel::DuplicateBeat(const FName SourceBeatId)
{
	USigilStoryAsset* StoryAsset = Asset.Get();
	const int32 SourceIndex = FindBeatIndex(SourceBeatId);
	if (!StoryAsset || SourceIndex == INDEX_NONE)
	{
		return NAME_None;
	}

	const FScopedTransaction Transaction(LOCTEXT("DuplicateStoryBeat", "Duplicate Story Beat"));
	StoryAsset->Modify();
	FSigilStoryBeatDefinition DuplicatedBeat = StoryAsset->Beats[SourceIndex];
	DuplicatedBeat.BeatId = MakeUniqueBeatId(SourceBeatId.ToString() + TEXT("_Copy"), false);
	DuplicateInstancedObjects(DuplicatedBeat);
	const FName DuplicatedBeatId = DuplicatedBeat.BeatId;
	StoryAsset->Beats.Add(MoveTemp(DuplicatedBeat));
	FinishStructuralChange();
	return DuplicatedBeatId;
}

bool FSigilStoryEditorModel::DeleteBeat(const FName BeatId)
{
	USigilStoryAsset* StoryAsset = Asset.Get();
	const int32 BeatIndex = FindBeatIndex(BeatId);
	if (!StoryAsset || BeatIndex == INDEX_NONE)
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteStoryBeat", "Delete Story Beat"));
	StoryAsset->Modify();
	StoryAsset->Beats.RemoveAt(BeatIndex);
	FinishStructuralChange();
	return true;
}

int32 FSigilStoryEditorModel::AddBeatObject(
	const FName BeatId,
	const ESigilStoryBeatObjectList List,
	UClass* ObjectClass,
	FText& OutError)
{
	OutError = FText::GetEmpty();
	USigilStoryAsset* StoryAsset = Asset.Get();
	const int32 BeatIndex = FindBeatIndex(BeatId);
	if (!StoryAsset || BeatIndex == INDEX_NONE)
	{
		OutError = LOCTEXT("StoryBeatMissingForObject", "The selected Story Beat no longer exists.");
		return INDEX_NONE;
	}
	if (!ObjectClass || ObjectClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
	{
		OutError = LOCTEXT("StoryBeatObjectClassInvalid", "Select a concrete Condition or Event class.");
		return INDEX_NONE;
	}

	const bool bConditionList = List == ESigilStoryBeatObjectList::EnterConditions;
	const UClass* RequiredBaseClass = bConditionList
		? USigilNarrativeCondition::StaticClass()
		: USigilNarrativeEvent::StaticClass();
	if (!ObjectClass->IsChildOf(RequiredBaseClass))
	{
		OutError = bConditionList
			? LOCTEXT("StoryConditionClassRequired", "Enter Conditions require a Condition class.")
			: LOCTEXT("StoryEventClassRequired", "Story Events require an Event class.");
		return INDEX_NONE;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddStoryBeatObject", "Add Story Beat Condition or Event"));
	StoryAsset->Modify();
	FSigilStoryBeatDefinition& Beat = StoryAsset->Beats[BeatIndex];
	int32 AddedIndex = INDEX_NONE;
	if (bConditionList)
	{
		USigilNarrativeCondition* NewCondition = NewObject<USigilNarrativeCondition>(
			StoryAsset, ObjectClass, NAME_None, RF_Transactional);
		AddedIndex = Beat.EnterConditions.Add(NewCondition);
	}
	else
	{
		USigilNarrativeEvent* NewEvent = NewObject<USigilNarrativeEvent>(
			StoryAsset, ObjectClass, NAME_None, RF_Transactional);
		TArray<TObjectPtr<USigilNarrativeEvent>>& Events =
			List == ESigilStoryBeatObjectList::EnterEvents ? Beat.EnterEvents : Beat.CompleteEvents;
		AddedIndex = Events.Add(NewEvent);
	}
	FinishStructuralChange();
	return AddedIndex;
}

bool FSigilStoryEditorModel::RemoveBeatObject(
	const FName BeatId,
	const ESigilStoryBeatObjectList List,
	const int32 ObjectIndex)
{
	USigilStoryAsset* StoryAsset = Asset.Get();
	const int32 BeatIndex = FindBeatIndex(BeatId);
	if (!StoryAsset || BeatIndex == INDEX_NONE)
	{
		return false;
	}

	FSigilStoryBeatDefinition& Beat = StoryAsset->Beats[BeatIndex];
	const bool bValidIndex = List == ESigilStoryBeatObjectList::EnterConditions
		? Beat.EnterConditions.IsValidIndex(ObjectIndex)
		: (List == ESigilStoryBeatObjectList::EnterEvents
			? Beat.EnterEvents.IsValidIndex(ObjectIndex)
			: Beat.CompleteEvents.IsValidIndex(ObjectIndex));
	if (!bValidIndex)
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveStoryBeatObject", "Remove Story Beat Condition or Event"));
	StoryAsset->Modify();
	if (List == ESigilStoryBeatObjectList::EnterConditions)
	{
		Beat.EnterConditions.RemoveAt(ObjectIndex);
	}
	else if (List == ESigilStoryBeatObjectList::EnterEvents)
	{
		Beat.EnterEvents.RemoveAt(ObjectIndex);
	}
	else
	{
		Beat.CompleteEvents.RemoveAt(ObjectIndex);
	}
	FinishStructuralChange();
	return true;
}

bool FSigilStoryEditorModel::ReconcileBeatEdit(
	const int32 BeatIndex,
	const FName PreviousBeatId,
	FText& OutError)
{
	OutError = FText::GetEmpty();
	USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset || !StoryAsset->Beats.IsValidIndex(BeatIndex))
	{
		OutError = LOCTEXT("EditedStoryBeatMissing", "The edited beat no longer exists.");
		return false;
	}

	FSigilStoryBeatDefinition& EditedBeat = StoryAsset->Beats[BeatIndex];
	const FName EditedBeatId = EditedBeat.BeatId;
	const bool bDuplicateBeatId = StoryAsset->Beats.ContainsByPredicate(
		[StoryAsset, BeatIndex, EditedBeatId](const FSigilStoryBeatDefinition& Beat)
		{
			return &Beat != &StoryAsset->Beats[BeatIndex] && Beat.BeatId == EditedBeatId;
		});
	if (EditedBeatId.IsNone() || bDuplicateBeatId)
	{
		EditedBeat.BeatId = PreviousBeatId;
		OutError = EditedBeatId.IsNone()
			? LOCTEXT("EmptyStoryBeatId", "BeatId must not be empty.")
			: FText::Format(
				LOCTEXT("DuplicateStoryBeatId", "BeatId {0} is already in use."),
				FText::FromName(EditedBeatId));
		FinishStructuralChange();
		return false;
	}

	FinishStructuralChange();
	return true;
}

FSimpleMulticastDelegate& FSigilStoryEditorModel::OnModelChanged()
{
	return ModelChanged;
}

USigilStoryAsset* FSigilStoryEditorModel::GetAsset() const
{
	return Asset.Get();
}

FName FSigilStoryEditorModel::MakeUniqueBeatId(
	const FString& BaseName,
	const bool bUsePaddedNumber) const
{
	if (bUsePaddedNumber)
	{
		for (int32 Number = 1; Number < MAX_int32; ++Number)
		{
			const FName Candidate(*FString::Printf(TEXT("%s_%03d"), *BaseName, Number));
			if (FindBeatIndex(Candidate) == INDEX_NONE)
			{
				return Candidate;
			}
		}
		return NAME_None;
	}

	const FName BaseCandidate(*BaseName);
	if (FindBeatIndex(BaseCandidate) == INDEX_NONE)
	{
		return BaseCandidate;
	}
	for (int32 Number = 2; Number < MAX_int32; ++Number)
	{
		const FName Candidate(*FString::Printf(TEXT("%s_%d"), *BaseName, Number));
		if (FindBeatIndex(Candidate) == INDEX_NONE)
		{
			return Candidate;
		}
	}
	return NAME_None;
}

void FSigilStoryEditorModel::DuplicateInstancedObjects(FSigilStoryBeatDefinition& Beat) const
{
	USigilStoryAsset* StoryAsset = Asset.Get();
	if (!StoryAsset)
	{
		return;
	}

	for (TObjectPtr<USigilNarrativeCondition>& Condition : Beat.EnterConditions)
	{
		Condition = Condition
			? DuplicateObject<USigilNarrativeCondition>(Condition, StoryAsset)
			: nullptr;
	}
	for (TObjectPtr<USigilNarrativeEvent>& Event : Beat.EnterEvents)
	{
		Event = Event ? DuplicateObject<USigilNarrativeEvent>(Event, StoryAsset) : nullptr;
	}
	for (TObjectPtr<USigilNarrativeEvent>& Event : Beat.CompleteEvents)
	{
		Event = Event ? DuplicateObject<USigilNarrativeEvent>(Event, StoryAsset) : nullptr;
	}
}

void FSigilStoryEditorModel::FinishStructuralChange()
{
	if (USigilStoryAsset* StoryAsset = Asset.Get())
	{
		StoryAsset->MarkPackageDirty();
	}
	ModelChanged.Broadcast();
}

#undef LOCTEXT_NAMESPACE
