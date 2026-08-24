// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilNarrativeEvent.h"
#include "SigilNarrativeSnapshotTestTypes.generated.h"

class USigilNarrativeCatalog;
class USigilDialogueSession;

UCLASS()
class USigilNarrativeSnapshotTestEvent final : public USigilNarrativeEvent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bExportDuringExecute = false;

	UPROPERTY()
	bool bImportDuringExecute = false;

	UPROPERTY()
	FString ImportJson;

	UPROPERTY()
	TObjectPtr<USigilNarrativeCatalog> ImportCatalog = nullptr;

	UPROPERTY()
	int32 ExecuteCount = 0;

	UPROPERTY()
	bool bLastExportResult = true;

	UPROPERTY()
	FString LastExportJson;

	UPROPERTY()
	bool bLastImportResult = true;

	UPROPERTY()
	TObjectPtr<USigilDialogueSession> DialogueSession = nullptr;

	UPROPERTY()
	FName ReentrantOptionId;

	UPROPERTY()
	bool bLastReentrantChooseResult = true;

	UPROPERTY()
	bool bCancelDialogueDuringExecute = false;

	virtual void Execute_Implementation(const FSigilNarrativeContext& Context) override;
};
