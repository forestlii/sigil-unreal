// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Common/SigilListView.h"

#include "UI/Common/SigilWidgetFactory.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif


USigilListView::USigilListView(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void USigilListView::ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const
{
	Super::ValidateCompiledDefaults(InCompileLog);
}
#endif

void USigilListView::SetEntryWidgetFactories(TArray<USigilWidgetFactory*> NewFactories)
{
	EntryWidgetFactories = NewFactories;
}

UUserWidget& USigilListView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
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
