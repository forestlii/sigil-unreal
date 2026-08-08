// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/Foundation/GUIS_TabListWidgetBase.h"

#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "CommonActivatableWidget.h"
#include "GUIS_LogChannels.h"
#include "Editor/WidgetCompilerLog.h"
#include "UI/Foundation/GUIS_TabDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_TabListWidgetBase)

void UGUIS_TabListWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UGUIS_TabListWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	SetupTabs();
}

void UGUIS_TabListWidgetBase::NativeDestruct()
{
	for (FGUIS_TabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
			TabInfo.CreatedTabContentWidget = nullptr;
		}
	}
	Super::NativeDestruct();
}

FGUIS_TabDescriptor::FGUIS_TabDescriptor()
{
	bHidden = false;
}

UGUIS_TabListWidgetBase::UGUIS_TabListWidgetBase()
{
	bAutoListenForInput = false;
	bDeferRebuildingTabList = true;
}

bool UGUIS_TabListWidgetBase::GetPreregisteredTabInfo(const FName TabNameId, FGUIS_TabDescriptor& OutTabInfo)
{
	const FGUIS_TabDescriptor* const FoundTabInfo = PreregisteredTabInfoArray.FindByPredicate([&](const FGUIS_TabDescriptor& TabInfo) -> bool
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

int32 UGUIS_TabListWidgetBase::GetPreregisteredTabIndex(FName TabNameId) const
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

bool UGUIS_TabListWidgetBase::FindPreregisteredTabInfo(const FName TabNameId, FGUIS_TabDescriptor& OutTabInfo)
{
	return GetPreregisteredTabInfo(TabNameId, OutTabInfo);
}

void UGUIS_TabListWidgetBase::SetTabHiddenState(FName TabNameId, bool bHidden)
{
	for (FGUIS_TabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.TabId == TabNameId)
		{
			TabInfo.bHidden = bHidden;
		}
	}
}

bool UGUIS_TabListWidgetBase::RegisterDynamicTab(const FGUIS_TabDescriptor& TabDescriptor)
{
	// If it's hidden we just ignore it.
	if (TabDescriptor.bHidden)
	{
		return true;
	}

	PendingTabLabelInfoMap.Add(TabDescriptor.TabId, TabDescriptor);

	return RegisterTab(TabDescriptor.TabId, TabDescriptor.TabButtonType.LoadSynchronous(), TabDescriptor.CreatedTabContentWidget);
}

void UGUIS_TabListWidgetBase::HandlePreLinkedSwitcherChanged()
{
	for (const FGUIS_TabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		// Remove tab content widget from linked switcher, as it is being disassociated.
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
		}
	}

	Super::HandlePreLinkedSwitcherChanged();
}

void UGUIS_TabListWidgetBase::HandlePostLinkedSwitcherChanged()
{
	if (!IsDesignTime() && GetCachedWidget().IsValid())
	{
		// Don't bother making tabs if we're in the designer or haven't been constructed yet
		SetupTabs();
	}

	Super::HandlePostLinkedSwitcherChanged();
}

void UGUIS_TabListWidgetBase::HandleTabCreation_Implementation(FName TabId, UCommonButtonBase* TabButton)
{
	FGUIS_TabDescriptor* TabInfoPtr = nullptr;

	FGUIS_TabDescriptor TabInfo;
	if (GetPreregisteredTabInfo(TabId, TabInfo))
	{
		TabInfoPtr = &TabInfo;
	}
	else
	{
		TabInfoPtr = PendingTabLabelInfoMap.Find(TabId);
	}

	if (TabButton->GetClass()->ImplementsInterface(UGUIS_TabButtonInterface::StaticClass()))
	{
		if (ensureMsgf(TabInfoPtr, TEXT("A tab button was created with id %s but no label info was specified. RegisterDynamicTab should be used over RegisterTab to provide label info."),
		               *TabId.ToString()))
		{
			IGUIS_TabButtonInterface::Execute_SetTabLabelInfo(TabButton, *TabInfoPtr);
		}
	}

	PendingTabLabelInfoMap.Remove(TabId);
}

bool UGUIS_TabListWidgetBase::IsFirstTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray[0].TabId;
	}

	return false;
}

bool UGUIS_TabListWidgetBase::IsLastTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray.Last().TabId;
	}

	return false;
}

bool UGUIS_TabListWidgetBase::IsTabVisible(FName TabId)
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

int32 UGUIS_TabListWidgetBase::GetVisibleTabCount()
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

void UGUIS_TabListWidgetBase::SetupTabs()
{
	for (FGUIS_TabDescriptor& TabInfo : PreregisteredTabInfoArray)
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
void UGUIS_TabListWidgetBase::PostLoad()
{
	if (!TabDefinitions_DEPRECATED.IsEmpty())
	{
		for (TObjectPtr<UDEPRECATED_GUIS_TabDefinition> Def : TabDefinitions_DEPRECATED)
		{
			if (Def)
			{
				FGUIS_TabDescriptor Tab;
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

void UGUIS_TabListWidgetBase::ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);
}
#endif
