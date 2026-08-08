// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/Foundation/SigilTabListWidgetBase.h"

#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "CommonActivatableWidget.h"
#include "SigilUILogChannels.h"
#include "Editor/WidgetCompilerLog.h"
#include "UI/Foundation/SigilTabDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilTabListWidgetBase)

void USigilTabListWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void USigilTabListWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	SetupTabs();
}

void USigilTabListWidgetBase::NativeDestruct()
{
	for (FSigilTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
			TabInfo.CreatedTabContentWidget = nullptr;
		}
	}
	Super::NativeDestruct();
}

FSigilTabDescriptor::FSigilTabDescriptor()
{
	bHidden = false;
}

USigilTabListWidgetBase::USigilTabListWidgetBase()
{
	bAutoListenForInput = false;
	bDeferRebuildingTabList = true;
}

bool USigilTabListWidgetBase::GetPreregisteredTabInfo(const FName TabNameId, FSigilTabDescriptor& OutTabInfo)
{
	const FSigilTabDescriptor* const FoundTabInfo = PreregisteredTabInfoArray.FindByPredicate([&](const FSigilTabDescriptor& TabInfo) -> bool
	{
		return TabInfo.TabId == TabNameId;
	});

	if (!FoundTabInfo)
	{
		return false;
	}

	OutTabInfo = *FoundTabInfo;
	return true;
}

int32 USigilTabListWidgetBase::GetPreregisteredTabIndex(FName TabNameId) const
{
	for (int32 i = 0; i < PreregisteredTabInfoArray.Num(); ++i)
	{
		if (PreregisteredTabInfoArray[i].TabId == TabNameId)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool USigilTabListWidgetBase::FindPreregisteredTabInfo(const FName TabNameId, FSigilTabDescriptor& OutTabInfo)
{
	return GetPreregisteredTabInfo(TabNameId, OutTabInfo);
}

void USigilTabListWidgetBase::SetTabHiddenState(FName TabNameId, bool bHidden)
{
	for (FSigilTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.TabId == TabNameId)
		{
			TabInfo.bHidden = bHidden;
		}
	}
}

bool USigilTabListWidgetBase::RegisterDynamicTab(const FSigilTabDescriptor& TabDescriptor)
{
	// If it's hidden we just ignore it.
	if (TabDescriptor.bHidden)
	{
		return true;
	}

	PendingTabLabelInfoMap.Add(TabDescriptor.TabId, TabDescriptor);

	return RegisterTab(TabDescriptor.TabId, TabDescriptor.TabButtonType.LoadSynchronous(), TabDescriptor.CreatedTabContentWidget);
}

void USigilTabListWidgetBase::HandlePreLinkedSwitcherChanged()
{
	for (const FSigilTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		// Remove tab content widget from linked switcher, as it is being disassociated.
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
		}
	}

	Super::HandlePreLinkedSwitcherChanged();
}

void USigilTabListWidgetBase::HandlePostLinkedSwitcherChanged()
{
	if (!IsDesignTime() && GetCachedWidget().IsValid())
	{
		// Don't bother making tabs if we're in the designer or haven't been constructed yet
		SetupTabs();
	}

	Super::HandlePostLinkedSwitcherChanged();
}

void USigilTabListWidgetBase::HandleTabCreation_Implementation(FName TabId, UCommonButtonBase* TabButton)
{
	FSigilTabDescriptor* TabInfoPtr = nullptr;

	FSigilTabDescriptor TabInfo;
	if (GetPreregisteredTabInfo(TabId, TabInfo))
	{
		TabInfoPtr = &TabInfo;
	}
	else
	{
		TabInfoPtr = PendingTabLabelInfoMap.Find(TabId);
	}

	if (TabButton->GetClass()->ImplementsInterface(USigilTabButtonInterface::StaticClass()))
	{
		if (ensureMsgf(TabInfoPtr, TEXT("A tab button was created with id %s but no label info was specified. RegisterDynamicTab should be used over RegisterTab to provide label info."),
		               *TabId.ToString()))
		{
			ISigilTabButtonInterface::Execute_SetTabLabelInfo(TabButton, *TabInfoPtr);
		}
	}

	PendingTabLabelInfoMap.Remove(TabId);
}

bool USigilTabListWidgetBase::IsFirstTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray[0].TabId;
	}

	return false;
}

bool USigilTabListWidgetBase::IsLastTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray.Last().TabId;
	}

	return false;
}

bool USigilTabListWidgetBase::IsTabVisible(FName TabId)
{
	if (const UCommonButtonBase* Button = GetTabButtonBaseByID(TabId))
	{
		const ESlateVisibility TabVisibility = Button->GetVisibility();
		return (TabVisibility == ESlateVisibility::Visible
			|| TabVisibility == ESlateVisibility::HitTestInvisible
			|| TabVisibility == ESlateVisibility::SelfHitTestInvisible);
	}

	return false;
}

int32 USigilTabListWidgetBase::GetVisibleTabCount()
{
	int32 Result = 0;
	const int32 TabCount = GetTabCount();
	for (int32 Index = 0; Index < TabCount; Index++)
	{
		if (IsTabVisible(GetTabIdAtIndex(Index)))
		{
			Result++;
		}
	}

	return Result;
}

void USigilTabListWidgetBase::SetupTabs()
{
	for (FSigilTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.bHidden)
		{
			continue;
		}

		// If the tab content hasn't been created already, create it.
		if (!TabInfo.CreatedTabContentWidget && !TabInfo.TabContentType.IsNull())
		{
			TabInfo.CreatedTabContentWidget = CreateWidget<UCommonUserWidget>(GetOwningPlayer(), TabInfo.TabContentType.LoadSynchronous());
			OnTabContentCreatedNative.Broadcast(TabInfo.TabId, Cast<UCommonUserWidget>(TabInfo.CreatedTabContentWidget));
			OnTabContentCreated.Broadcast(TabInfo.TabId, Cast<UCommonUserWidget>(TabInfo.CreatedTabContentWidget));
		}

		if (UCommonAnimatedSwitcher* CurrentLinkedSwitcher = GetLinkedSwitcher())
		{
			// Add the tab content to the newly linked switcher.
			if (!CurrentLinkedSwitcher->HasChild(TabInfo.CreatedTabContentWidget))
			{
				CurrentLinkedSwitcher->AddChild(TabInfo.CreatedTabContentWidget);
			}
		}

		// If the tab is not already registered, register it.
		if (GetTabButtonBaseByID(TabInfo.TabId) == nullptr)
		{
			RegisterTab(TabInfo.TabId, TabInfo.TabButtonType.LoadSynchronous(), TabInfo.CreatedTabContentWidget);
		}
	}
}

#if WITH_EDITOR
void USigilTabListWidgetBase::PostLoad()
{
	if (!TabDefinitions_DEPRECATED.IsEmpty())
	{
		for (TObjectPtr<UDEPRECATED_SigilTabDefinition> Def : TabDefinitions_DEPRECATED)
		{
			if (Def)
			{
				FSigilTabDescriptor Tab;
				Tab.TabId = Def->TabId;
				Tab.IconBrush = Def->IconBrush;
				Tab.TabButtonType = Def->TabButtonType;
				Tab.TabText = Def->TabText;
				PreregisteredTabInfoArray.Add(Tab);
			}
		}
		TabDefinitions_DEPRECATED.Empty();
	}

	Super::PostLoad();
}

void USigilTabListWidgetBase::ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);
}
#endif
