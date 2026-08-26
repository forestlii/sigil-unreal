// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilQuestAsset.h"
#include "UObject/StructOnScope.h"

class FQuestStateStructOnScope final : public FStructOnScope
{
public:
	FQuestStateStructOnScope(USigilQuestAsset* InAsset, int32 InStateIndex);

	virtual uint8* GetStructMemory() override;
	virtual const uint8* GetStructMemory() const override;
	virtual const UScriptStruct* GetStruct() const override;
	virtual UPackage* GetPackage() const override;
	virtual void SetPackage(UPackage* InPackage) override;
	virtual bool IsValid() const override;

private:
	TWeakObjectPtr<USigilQuestAsset> Asset;
	int32 StateIndex = INDEX_NONE;
};

class FSigilQuestEditorModel
{
public:
	explicit FSigilQuestEditorModel(USigilQuestAsset* InAsset);

	TArray<FName> GetFilteredStateIds(const FString& Filter) const;
	int32 FindStateIndex(FName StateId) const;
	FName AddState(ESigilQuestStateType StateType);
	FName DuplicateState(FName SourceStateId);
	bool CanDeleteState(FName StateId, FText& OutReason) const;
	bool DeleteState(FName StateId, FText& OutReason);
	bool SetInitialState(FName StateId);
	bool ReconcileStateEdit(
		int32 StateIndex,
		FName PreviousStateId,
		ESigilQuestStateType PreviousStateType,
		FText& OutError);
	FSimpleMulticastDelegate& OnModelChanged();
	USigilQuestAsset* GetAsset() const;

private:
	FName MakeUniqueStateId(const FString& BaseName, bool bUsePaddedNumber) const;
	void DuplicateInstancedObjects(FSigilQuestState& State) const;
	void FinishStructuralChange();

	TWeakObjectPtr<USigilQuestAsset> Asset;
	FSimpleMulticastDelegate ModelChanged;
};
