// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilStoryAsset.h"
#include "UObject/StructOnScope.h"

class FStoryBeatStructOnScope final : public FStructOnScope
{
public:
	FStoryBeatStructOnScope(USigilStoryAsset* InAsset, int32 InBeatIndex);

	virtual uint8* GetStructMemory() override;
	virtual const uint8* GetStructMemory() const override;
	virtual const UScriptStruct* GetStruct() const override;
	virtual UPackage* GetPackage() const override;
	virtual void SetPackage(UPackage* InPackage) override;
	virtual bool IsValid() const override;

private:
	TWeakObjectPtr<USigilStoryAsset> Asset;
	int32 BeatIndex = INDEX_NONE;
};

enum class ESigilStoryBeatObjectList : uint8
{
	EnterConditions,
	EnterEvents,
	CompleteEvents
};

class FSigilStoryEditorModel
{
public:
	explicit FSigilStoryEditorModel(USigilStoryAsset* InAsset);

	bool SetStoryId(FName NewStoryId, FText& OutError);
	TArray<FName> GetFilteredBeatIds(const FString& Filter) const;
	int32 FindBeatIndex(FName BeatId) const;
	FName AddBeat();
	FName DuplicateBeat(FName SourceBeatId);
	bool DeleteBeat(FName BeatId);
	int32 AddBeatObject(
		FName BeatId,
		ESigilStoryBeatObjectList List,
		UClass* ObjectClass,
		FText& OutError);
	bool RemoveBeatObject(FName BeatId, ESigilStoryBeatObjectList List, int32 ObjectIndex);
	bool ReconcileBeatEdit(int32 BeatIndex, FName PreviousBeatId, FText& OutError);
	FSimpleMulticastDelegate& OnModelChanged();
	USigilStoryAsset* GetAsset() const;

private:
	FName MakeUniqueBeatId(const FString& BaseName, bool bUsePaddedNumber) const;
	void DuplicateInstancedObjects(FSigilStoryBeatDefinition& Beat) const;
	void FinishStructuralChange();

	TWeakObjectPtr<USigilStoryAsset> Asset;
	FSimpleMulticastDelegate ModelChanged;
};
