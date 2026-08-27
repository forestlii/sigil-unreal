// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilNarrativeSnapshotTestTypes.h"

#include "SigilDialogueSession.h"
#include "SigilNarrativeCatalog.h"
#include "SigilNarrativeSubsystem.h"

void USigilNarrativeSnapshotTestEvent::Execute_Implementation(const FSigilNarrativeContext& Context)
{
	++ExecuteCount;
	if (!Context.NarrativeSubsystem)
	{
		return;
	}

	if (bExportDuringExecute)
	{
		bLastExportResult = Context.NarrativeSubsystem->ExportSnapshotJson(LastExportJson);
	}
	if (bImportDuringExecute)
	{
		bLastImportResult = Context.NarrativeSubsystem->ImportSnapshotJson(ImportJson, ImportCatalog);
	}
	if (DialogueSession && ExecuteCount == 1 && !ReentrantOptionId.IsNone())
	{
		bLastReentrantChooseResult = DialogueSession->Choose(ReentrantOptionId);
	}
	if (DialogueSession && bCancelDialogueDuringExecute)
	{
		DialogueSession->Cancel();
	}
}
