// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Common/GUIS_TileView.h"

#include "UI/Common/GUIS_WidgetFactory.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif


UGUIS_TileView::UGUIS_TileView(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void UGUIS_TileView::ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const
{
	Super::ValidateCompiledDefaults(InCompileLog);
}
#endif

void UGUIS_TileView::SetEntryWidgetFactories(TArray<UGUIS_WidgetFactory*> NewFactories)
{
	EntryWidgetFactories = NewFactories;
}

UUserWidget& UGUIS_TileView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
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
