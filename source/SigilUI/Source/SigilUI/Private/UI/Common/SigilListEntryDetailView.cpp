// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Common/SigilListEntryDetailView.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UI/Common/SigilListEntryDetailSection.h"
#include "UI/Common/SigilDetailSectionsBuilder.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilListEntryDetailView)

#define LOCTEXT_NAMESPACE "EntryDetailsView"

USigilListEntryDetailView::USigilListEntryDetailView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , ExtensionWidgetPool(*this)
{
}

void USigilListEntryDetailView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ExtensionWidgetPool.ReleaseAllSlateResources();
}

void USigilListEntryDetailView::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!IsDesignTime())
	{
		SetListItemObject(nullptr);
	}
}

void USigilListEntryDetailView::NativeConstruct()
{
	Super::NativeConstruct();
}

void USigilListEntryDetailView::SetListItemObject(UObject* InListItemObject)
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
			TArray<TSoftClassPtr<USigilListEntryDetailSection>> SectionClasses;
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
			for (TSoftClassPtr<USigilListEntryDetailSection> SoftClassPtr : SectionClasses)
			{
				bEverythingAlreadyLoaded &= SoftClassPtr.IsValid();
				SectionPaths.Add(SoftClassPtr.ToSoftObjectPath());
			}

			if (bEverythingAlreadyLoaded)
			{
				for (TSoftClassPtr<USigilListEntryDetailSection> SoftClassPtr : SectionClasses)
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
						                                      for (TSoftClassPtr<USigilListEntryDetailSection> SoftClassPtr : SectionClasses)
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

void USigilListEntryDetailView::SetSectionsBuilder(USigilDetailSectionsBuilder* NewBuilder)
{
	SectionsBuilder = NewBuilder;
}

void USigilListEntryDetailView::CreateDetailsExtension(UObject* InData, TSubclassOf<USigilListEntryDetailSection> SectionClass)
{
	if (InData && SectionClass)
	{
		if (USigilListEntryDetailSection* Section = ExtensionWidgetPool.GetOrCreateInstance(SectionClass))
		{
			Section->SetListItemObject(InData);
			UVerticalBoxSlot* ExtensionSlot = Box_DetailSections->AddChildToVerticalBox(Section);
			ExtensionSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

#undef LOCTEXT_NAMESPACE
