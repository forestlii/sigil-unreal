// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Common/SigilTileView.h"

#include "UI/Common/SigilWidgetFactory.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif


USigilTileView::USigilTileView(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void USigilTileView::ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const
{
	Super::ValidateCompiledDefaults(InCompileLog);
}
#endif

void USigilTileView::SetEntryWidgetFactories(TArray<USigilWidgetFactory*> NewFactories)
{
	EntryWidgetFactories = NewFactories;
}

UUserWidget& USigilTileView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSubclassOf<UUserWidget> WidgetClass = DesiredEntryClass;

	for (const USigilWidgetFactory* Factory : EntryWidgetFactories)
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
