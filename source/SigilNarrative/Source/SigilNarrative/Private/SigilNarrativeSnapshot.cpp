// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilNarrativeSubsystem.h"

#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SigilNarrativeCatalog.h"

namespace
{
	constexpr int32 SnapshotSchemaVersion = 1;

	void SortNames(TArray<FName>& Names)
	{
		Names.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});
	}

	const TCHAR* QuestStatusToString(const ESigilQuestStatus Status)
	{
		switch (Status)
		{
		case ESigilQuestStatus::Active:
			return TEXT("Active");

		case ESigilQuestStatus::Succeeded:
			return TEXT("Succeeded");

		case ESigilQuestStatus::Failed:
			return TEXT("Failed");

		default:
			return nullptr;
		}
	}

	bool TryParseQuestStatus(const FString& StatusString, ESigilQuestStatus& OutStatus)
	{
		if (StatusString == TEXT("Active"))
		{
			OutStatus = ESigilQuestStatus::Active;
			return true;
		}
		if (StatusString == TEXT("Succeeded"))
		{
			OutStatus = ESigilQuestStatus::Succeeded;
			return true;
		}
		if (StatusString == TEXT("Failed"))
		{
			OutStatus = ESigilQuestStatus::Failed;
			return true;
		}

		return false;
	}

	bool DoesQuestStatusMatchState(const ESigilQuestStatus Status, const ESigilQuestStateType StateType)
	{
		switch (Status)
		{
		case ESigilQuestStatus::Active:
			return StateType == ESigilQuestStateType::Regular;

		case ESigilQuestStatus::Succeeded:
			return StateType == ESigilQuestStateType::Success;

		case ESigilQuestStatus::Failed:
			return StateType == ESigilQuestStateType::Failure;

		default:
			return false;
		}
	}

	bool TryGetObject(const TSharedPtr<FJsonValue>& JsonValue, TSharedPtr<FJsonObject>& OutObject)
	{
		const TSharedPtr<FJsonObject>* ObjectPointer = nullptr;
		if (!JsonValue.IsValid() || !JsonValue->TryGetObject(ObjectPointer) || !ObjectPointer || !ObjectPointer->IsValid())
		{
			return false;
		}

		OutObject = *ObjectPointer;
		return true;
	}

	bool TryGetStrictStringValue(const TSharedPtr<FJsonValue>& JsonValue, FString& OutString)
	{
		return JsonValue.IsValid()
			&& JsonValue->Type == EJson::String
			&& JsonValue->TryGetString(OutString);
	}

	bool TryGetStrictStringField(const FJsonObject& JsonObject, const TCHAR* FieldName, FString& OutString)
	{
		return TryGetStrictStringValue(JsonObject.TryGetField(FieldName), OutString);
	}

	bool IsSafeRequiredNameString(const FString& NameString)
	{
		return !NameString.IsEmpty() && NameString.Len() < NAME_SIZE;
	}

	bool TryGetRequiredNameField(const FJsonObject& JsonObject, const TCHAR* FieldName, FName& OutName)
	{
		FString NameString;
		if (!TryGetStrictStringField(JsonObject, FieldName, NameString) || !IsSafeRequiredNameString(NameString))
		{
			return false;
		}

		OutName = FName(*NameString);
		return !OutName.IsNone();
	}

	bool TryGetStrictInt32Field(const FJsonObject& JsonObject, const TCHAR* FieldName, int32& OutValue)
	{
		const TSharedPtr<FJsonValue> JsonValue = JsonObject.TryGetField(FieldName);
		double Number = 0.0;
		if (!JsonValue.IsValid()
			|| JsonValue->Type != EJson::Number
			|| !JsonValue->TryGetNumber(Number)
			|| !FMath::IsFinite(Number)
			|| FMath::TruncToDouble(Number) != Number
			|| Number < static_cast<double>(TNumericLimits<int32>::Min())
			|| Number > static_cast<double>(TNumericLimits<int32>::Max()))
		{
			return false;
		}

		OutValue = static_cast<int32>(Number);
		return true;
	}

	bool ValidateQuestRuntimeState(const FName QuestId, const FSigilQuestRuntimeState& RuntimeState)
	{
		FText ValidationError;
		if (!RuntimeState.QuestAsset
			|| RuntimeState.bCallbackInProgress
			|| RuntimeState.QuestAsset->QuestId != QuestId
			|| !RuntimeState.QuestAsset->ValidateDefinition(ValidationError))
		{
			return false;
		}

		const FSigilQuestState* State = RuntimeState.QuestAsset->FindState(RuntimeState.CurrentStateId);
		if (!State || !DoesQuestStatusMatchState(RuntimeState.Status, State->StateType))
		{
			return false;
		}

		if (RuntimeState.Status != ESigilQuestStatus::Active)
		{
			return RuntimeState.TaskProgress.IsEmpty();
		}
		if (RuntimeState.TaskProgress.Num() != State->Tasks.Num())
		{
			return false;
		}

		for (const FSigilQuestTaskDefinition& Task : State->Tasks)
		{
			const int32* Count = RuntimeState.TaskProgress.Find(Task.TaskId);
			if (!Count || *Count < 0 || *Count > Task.RequiredCount)
			{
				return false;
			}
		}

		return true;
	}

	bool ValidateStoryRuntimeState(const FName StoryId, const FSigilStoryRuntimeState& RuntimeState)
	{
		FText ValidationError;
		if (!RuntimeState.StoryAsset
			|| RuntimeState.bCallbackInProgress
			|| RuntimeState.StoryAsset->StoryId != StoryId
			|| !RuntimeState.StoryAsset->ValidateDefinition(ValidationError))
		{
			return false;
		}

		if (!RuntimeState.ActiveBeatId.IsNone())
		{
			if (!RuntimeState.StoryAsset->FindBeat(RuntimeState.ActiveBeatId)
				|| RuntimeState.CompletedBeatIds.Contains(RuntimeState.ActiveBeatId))
			{
				return false;
			}
		}

		for (const FName CompletedBeatId : RuntimeState.CompletedBeatIds)
		{
			if (CompletedBeatId.IsNone() || !RuntimeState.StoryAsset->FindBeat(CompletedBeatId))
			{
				return false;
			}
		}

		return true;
	}
}

bool USigilNarrativeSubsystem::ExportSnapshotJson(FString& OutJson) const
{
	OutJson.Reset();
	if (ActiveDialogueCallbackCount > 0)
	{
		return false;
	}

	for (const TPair<FName, FSigilQuestRuntimeState>& QuestPair : QuestStates)
	{
		if (!ValidateQuestRuntimeState(QuestPair.Key, QuestPair.Value))
		{
			OutJson.Reset();
			return false;
		}
	}
	for (const TPair<FName, FSigilStoryRuntimeState>& StoryPair : StoryStates)
	{
		if (!ValidateStoryRuntimeState(StoryPair.Key, StoryPair.Value))
		{
			OutJson.Reset();
			return false;
		}
	}

	TArray<FName> SortedFlags = Flags.Array();
	TArray<FName> SortedQuestIds;
	TArray<FName> SortedStoryIds;
	QuestStates.GenerateKeyArray(SortedQuestIds);
	StoryStates.GenerateKeyArray(SortedStoryIds);
	SortNames(SortedFlags);
	SortNames(SortedQuestIds);
	SortNames(SortedStoryIds);

	using FSnapshotWriter = TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;
	TSharedRef<FSnapshotWriter> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJson);
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("SchemaVersion"), SnapshotSchemaVersion);

	Writer->WriteArrayStart(TEXT("Flags"));
	for (const FName Flag : SortedFlags)
	{
		Writer->WriteValue(Flag.ToString());
	}
	Writer->WriteArrayEnd();

	Writer->WriteArrayStart(TEXT("Quests"));
	for (const FName QuestId : SortedQuestIds)
	{
		const FSigilQuestRuntimeState& RuntimeState = QuestStates.FindChecked(QuestId);
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("QuestId"), QuestId.ToString());
		Writer->WriteValue(TEXT("Status"), QuestStatusToString(RuntimeState.Status));
		Writer->WriteValue(TEXT("CurrentStateId"), RuntimeState.CurrentStateId.ToString());

		TArray<FName> SortedTaskIds;
		RuntimeState.TaskProgress.GenerateKeyArray(SortedTaskIds);
		SortNames(SortedTaskIds);
		Writer->WriteArrayStart(TEXT("Tasks"));
		for (const FName TaskId : SortedTaskIds)
		{
			Writer->WriteObjectStart();
			Writer->WriteValue(TEXT("TaskId"), TaskId.ToString());
			Writer->WriteValue(TEXT("Count"), RuntimeState.TaskProgress.FindChecked(TaskId));
			Writer->WriteObjectEnd();
		}
		Writer->WriteArrayEnd();
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();

	Writer->WriteArrayStart(TEXT("Stories"));
	for (const FName StoryId : SortedStoryIds)
	{
		const FSigilStoryRuntimeState& RuntimeState = StoryStates.FindChecked(StoryId);
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("StoryId"), StoryId.ToString());
		Writer->WriteValue(
			TEXT("ActiveBeatId"),
			RuntimeState.ActiveBeatId.IsNone() ? FString() : RuntimeState.ActiveBeatId.ToString());

		TArray<FName> SortedCompletedBeatIds = RuntimeState.CompletedBeatIds.Array();
		SortNames(SortedCompletedBeatIds);
		Writer->WriteArrayStart(TEXT("CompletedBeatIds"));
		for (const FName CompletedBeatId : SortedCompletedBeatIds)
		{
			Writer->WriteValue(CompletedBeatId.ToString());
		}
		Writer->WriteArrayEnd();
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd();

	if (!Writer->Close())
	{
		OutJson.Reset();
		return false;
	}

	return true;
}

bool USigilNarrativeSubsystem::ImportSnapshotJson(const FString& JsonText, USigilNarrativeCatalog* Catalog)
{
	if (ActiveDialogueCallbackCount > 0)
	{
		return false;
	}

	for (const TPair<FName, FSigilQuestRuntimeState>& QuestPair : QuestStates)
	{
		if (QuestPair.Value.bCallbackInProgress)
		{
			return false;
		}
	}
	for (const TPair<FName, FSigilStoryRuntimeState>& StoryPair : StoryStates)
	{
		if (StoryPair.Value.bCallbackInProgress)
		{
			return false;
		}
	}

	FText CatalogError;
	if (!Catalog || !Catalog->ValidateDefinition(CatalogError))
	{
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return false;
	}

	int32 SchemaVersion = 0;
	if (!TryGetStrictInt32Field(*RootObject, TEXT("SchemaVersion"), SchemaVersion)
		|| SchemaVersion != SnapshotSchemaVersion)
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* FlagValues = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* QuestValues = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* StoryValues = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("Flags"), FlagValues)
		|| !RootObject->TryGetArrayField(TEXT("Quests"), QuestValues)
		|| !RootObject->TryGetArrayField(TEXT("Stories"), StoryValues))
	{
		return false;
	}

	TSet<FName> ImportedFlags;
	TMap<FName, FSigilQuestRuntimeState> ImportedQuestStates;
	TMap<FName, FSigilStoryRuntimeState> ImportedStoryStates;

	for (const TSharedPtr<FJsonValue>& FlagValue : *FlagValues)
	{
		FString FlagString;
		if (!TryGetStrictStringValue(FlagValue, FlagString) || !IsSafeRequiredNameString(FlagString))
		{
			return false;
		}

		const FName Flag(*FlagString);
		if (Flag.IsNone() || ImportedFlags.Contains(Flag))
		{
			return false;
		}
		ImportedFlags.Add(Flag);
	}

	for (const TSharedPtr<FJsonValue>& QuestValue : *QuestValues)
	{
		TSharedPtr<FJsonObject> QuestObject;
		if (!TryGetObject(QuestValue, QuestObject))
		{
			return false;
		}

		FName QuestId;
		FName CurrentStateId;
		FString StatusString;
		ESigilQuestStatus Status = ESigilQuestStatus::NotStarted;
		if (!TryGetRequiredNameField(*QuestObject, TEXT("QuestId"), QuestId)
			|| ImportedQuestStates.Contains(QuestId)
			|| !TryGetStrictStringField(*QuestObject, TEXT("Status"), StatusString)
			|| !TryParseQuestStatus(StatusString, Status)
			|| !TryGetRequiredNameField(*QuestObject, TEXT("CurrentStateId"), CurrentStateId))
		{
			return false;
		}

		USigilQuestAsset* QuestAsset = Catalog->FindQuest(QuestId);
		const FSigilQuestState* QuestState = QuestAsset ? QuestAsset->FindState(CurrentStateId) : nullptr;
		if (!QuestState || !DoesQuestStatusMatchState(Status, QuestState->StateType))
		{
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* TaskValues = nullptr;
		if (!QuestObject->TryGetArrayField(TEXT("Tasks"), TaskValues))
		{
			return false;
		}

		TMap<FName, int32> TaskProgress;
		for (const TSharedPtr<FJsonValue>& TaskValue : *TaskValues)
		{
			TSharedPtr<FJsonObject> TaskObject;
			FName TaskId;
			int32 Count = 0;
			if (!TryGetObject(TaskValue, TaskObject)
				|| !TryGetRequiredNameField(*TaskObject, TEXT("TaskId"), TaskId)
				|| TaskProgress.Contains(TaskId)
				|| !TryGetStrictInt32Field(*TaskObject, TEXT("Count"), Count))
			{
				return false;
			}
			TaskProgress.Add(TaskId, Count);
		}

		if (Status == ESigilQuestStatus::Active)
		{
			if (TaskProgress.Num() != QuestState->Tasks.Num())
			{
				return false;
			}
			for (const FSigilQuestTaskDefinition& Task : QuestState->Tasks)
			{
				const int32* Count = TaskProgress.Find(Task.TaskId);
				if (!Count || *Count < 0 || *Count > Task.RequiredCount)
				{
					return false;
				}
			}
		}
		else if (!TaskProgress.IsEmpty())
		{
			return false;
		}

		FSigilQuestRuntimeState ImportedRuntimeState;
		ImportedRuntimeState.QuestAsset = QuestAsset;
		ImportedRuntimeState.Status = Status;
		ImportedRuntimeState.CurrentStateId = CurrentStateId;
		ImportedRuntimeState.TaskProgress = MoveTemp(TaskProgress);
		ImportedRuntimeState.bCallbackInProgress = false;
		ImportedQuestStates.Add(QuestId, MoveTemp(ImportedRuntimeState));
	}

	for (const TSharedPtr<FJsonValue>& StoryValue : *StoryValues)
	{
		TSharedPtr<FJsonObject> StoryObject;
		FName StoryId;
		FString ActiveBeatString;
		if (!TryGetObject(StoryValue, StoryObject)
			|| !TryGetRequiredNameField(*StoryObject, TEXT("StoryId"), StoryId)
			|| ImportedStoryStates.Contains(StoryId)
			|| !TryGetStrictStringField(*StoryObject, TEXT("ActiveBeatId"), ActiveBeatString))
		{
			return false;
		}

		USigilStoryAsset* StoryAsset = Catalog->FindStory(StoryId);
		if (!StoryAsset)
		{
			return false;
		}

		FName ActiveBeatId;
		if (!ActiveBeatString.IsEmpty())
		{
			if (ActiveBeatString.Len() >= NAME_SIZE)
			{
				return false;
			}
			ActiveBeatId = FName(*ActiveBeatString);
			if (ActiveBeatId.IsNone() || !StoryAsset->FindBeat(ActiveBeatId))
			{
				return false;
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* CompletedBeatValues = nullptr;
		if (!StoryObject->TryGetArrayField(TEXT("CompletedBeatIds"), CompletedBeatValues))
		{
			return false;
		}

		TSet<FName> CompletedBeatIds;
		for (const TSharedPtr<FJsonValue>& CompletedBeatValue : *CompletedBeatValues)
		{
			FString CompletedBeatString;
			if (!TryGetStrictStringValue(CompletedBeatValue, CompletedBeatString)
				|| !IsSafeRequiredNameString(CompletedBeatString))
			{
				return false;
			}

			const FName CompletedBeatId(*CompletedBeatString);
			if (CompletedBeatId.IsNone()
				|| CompletedBeatIds.Contains(CompletedBeatId)
				|| !StoryAsset->FindBeat(CompletedBeatId))
			{
				return false;
			}
			CompletedBeatIds.Add(CompletedBeatId);
		}

		if (!ActiveBeatId.IsNone() && CompletedBeatIds.Contains(ActiveBeatId))
		{
			return false;
		}

		FSigilStoryRuntimeState ImportedRuntimeState;
		ImportedRuntimeState.StoryAsset = StoryAsset;
		ImportedRuntimeState.ActiveBeatId = ActiveBeatId;
		ImportedRuntimeState.CompletedBeatIds = MoveTemp(CompletedBeatIds);
		ImportedRuntimeState.bCallbackInProgress = false;
		ImportedStoryStates.Add(StoryId, MoveTemp(ImportedRuntimeState));
	}

	Flags = MoveTemp(ImportedFlags);
	QuestStates = MoveTemp(ImportedQuestStates);
	StoryStates = MoveTemp(ImportedStoryStates);
	return true;
}
