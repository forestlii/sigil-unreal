// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Common/GUIS_ListEntryDetailView.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UI/Common/GUIS_ListEntryDetailSection.h"
#include "UI/Common/GUIS_DetailSectionsBuilder.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_ListEntryDetailView)

#define LOCTEXT_NAMESPACE "EntryDetailsView"

UGUIS_ListEntryDetailView::UGUIS_ListEntryDetailView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , ExtensionWidgetPool(*this)
{
}

void UGUIS_ListEntryDetailView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ExtensionWidgetPool.ReleaseAllSlateResources();
}

void UGUIS_ListEntryDetailView::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!IsDesignTime())
	{
		SetListItemObject(nullptr);
	}
}

void UGUIS_ListEntryDetailView::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGUIS_ListEntryDetailView::SetListItemObject(UObject* InListItemObject)
{
	// Ignore requests to show the same setting multiple times in a row. 
	if (InListItemObject && InListItemObject == CurrentListItemObject)
	{
		return;
	}

	CurrentListItemObject = InListItemObject;

	if (Box_DetailSections)
	{
		// First release the widgets back into the pool.
		for (UWidget* ChildExtension : Box_DetailSections->GetAllChildren())
		{
			ExtensionWidgetPool.Release(Cast<UUserWidget>(ChildExtension));
		}

		// Remove the widgets from their container.
		Box_DetailSections->ClearChildren();

		if (InListItemObject)
		{
			TArray<TSoftClassPtr<UGUIS_ListEntryDetailSection>> SectionClasses;
			if (SectionsBuilder)
			{
				SectionClasses = SectionsBuilder->GatherDetailSections(InListItemObject);
			}

			if (StreamingHandle.IsValid())
			{
				StreamingHandle->CancelHandle();
			}

			bool bEverythingAlreadyLoaded = true;

			TArray<FSoftObjectPath> SectionPaths;
			SectionPaths.Reserve(SectionClasses.Num());
			for (TSoftClassPtr<UGUIS_ListEntryDetailSection> SoftClassPtr : SectionClasses)
			{
				bEverythingAlreadyLoaded &= SoftClassPtr.IsValid();
				SectionPaths.Add(SoftClassPtr.ToSoftObjectPath());
			}

			if (bEverythingAlreadyLoaded)
			{
				for (TSoftClassPtr<UGUIS_ListEntryDetailSection> SoftClassPtr : SectionClasses)
				{
					CreateDetailsExtension(InListItemObject, SoftClassPtr.Get());
				}

				ExtensionWidgetPool.ReleaseInactiveSlateResources();
			}
			else
			{
				TWeakObjectPtr<UObject> SettingPtr = InListItemObject;

				StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
					MoveTemp(SectionPaths),
					FStreamableDelegate::CreateWeakLambda(this, [this, SettingPtr, SectionClasses]
					                                      {
						                                      for (TSoftClassPtr<UGUIS_ListEntryDetailSection> SoftClassPtr : SectionClasses)
						                                      {
							                                      CreateDetailsExtension(SettingPtr.Get(), SoftClassPtr.Get());
						                                      }

						                                      ExtensionWidgetPool.ReleaseInactiveSlateResources();
					                                      }
					));
			}
		}
	}
}

void UGUIS_ListEntryDetailView::SetSectionsBuilder(UGUIS_DetailSectionsBuilder* NewBuilder)
{
	SectionsBuilder = NewBuilder;
}

void UGUIS_ListEntryDetailView::CreateDetailsExtension(UObject* InData, TSubclassOf<UGUIS_ListEntryDetailSection> SectionClass)
{
	if (InData && SectionClass)
	{
		if (UGUIS_ListEntryDetailSection* Section = ExtensionWidgetPool.GetOrCreateInstance(SectionClass))
		{
			Section->SetListItemObject(InData);
			UVerticalBoxSlot* ExtensionSlot = Box_DetailSections->AddChildToVerticalBox(Section);
			ExtensionSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

#undef LOCTEXT_NAMESPACE
