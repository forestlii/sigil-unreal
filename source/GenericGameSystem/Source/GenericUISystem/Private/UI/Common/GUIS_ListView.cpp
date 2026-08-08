// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Common/GUIS_ListView.h"

#include "UI/Common/GUIS_WidgetFactory.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif


UGUIS_ListView::UGUIS_ListView(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void UGUIS_ListView::ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const
{
	Super::ValidateCompiledDefaults(InCompileLog);
}
#endif

void UGUIS_ListView::SetEntryWidgetFactories(TArray<UGUIS_WidgetFactory*> NewFactories)
{
	EntryWidgetFactories = NewFactories;
}

UUserWidget& UGUIS_ListView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSubclassOf<UUserWidget> WidgetClass = DesiredEntryClass;

	for (const UGUIS_WidgetFactory* Factory : EntryWidgetFactories)
	{
		if (Factory)
		{
			if (const TSubclassOf<UUserWidget> EntryClass = Factory->FindWidgetClassForData(Item))
			{
				WidgetClass = EntryClass;
				break;
			}
		}
	}

	return Super::OnGenerateEntryWidgetInternal(Item, WidgetClass, OwnerTable);
}
