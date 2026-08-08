// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/GUIS_GameUIPolicy.h"
#include "UI/GUIS_GameUISubsystem.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "GUIS_LogChannels.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/GUIS_GameUIContext.h"
#include "UI/GUIS_GameUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_GameUIPolicy)

// Static
UGUIS_GameUIPolicy* UGUIS_GameUIPolicy::GetGameUIPolicy(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGUIS_GameUISubsystem* UIManager = UGameInstance::GetSubsystem<UGUIS_GameUISubsystem>(GameInstance))
			{
				return UIManager->GetCurrentUIPolicy();
			}
		}
	}

	return nullptr;
}

UGUIS_GameUISubsystem* UGUIS_GameUIPolicy::GetOwningSubsystem() const
{
	return Cast<UGUIS_GameUISubsystem>(GetOuter());
}

UWorld* UGUIS_GameUIPolicy::GetWorld() const
{
	if (UGUIS_GameUISubsystem* Subsystem = GetOwningSubsystem())
	{
		return Subsystem->GetGameInstance()->GetWorld();
	}
	return nullptr;
}

UGUIS_GameUILayout* UGUIS_GameUIPolicy::GetRootLayout(const ULocalPlayer* LocalPlayer) const
{
	const FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}

UGUIS_GameUIContext* UGUIS_GameUIPolicy::GetContext(const ULocalPlayer* LocalPlayer, TSubclassOf<UGUIS_GameUIContext> ContextClass)
{
	if (const FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		for (int32 i = 0; i < LayoutInfo->Contexts.Num(); i++)
		{
			if (LayoutInfo->Contexts[i] && LayoutInfo->Contexts[i]->GetClass() == ContextClass)
			{
				return LayoutInfo->Contexts[i];
			}
		}
	}
	return nullptr;
}

bool UGUIS_GameUIPolicy::AddContext(const ULocalPlayer* LocalPlayer, UGUIS_GameUIContext* NewContext)
{
	if (FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		if (const UObject* ExistingContext = GetContext(LocalPlayer, NewContext->GetClass()))
		{
			UE_LOG(LogGUIS, Warning, TEXT("[%s] is trying to add repeat context of type(%s) for %s, which is not allowed!"), *GetName(), *NewContext->GetClass()->GetName(), *GetNameSafe(LocalPlayer));
			return false;
		}
		LayoutInfo->Contexts.Add(NewContext);
		UE_LOG(LogGUIS, Verbose, TEXT("[%s] registered context of type(%s) for %s."), *GetName(), *NewContext->GetClass()->GetName(), *GetNameSafe(LocalPlayer));
		return true;
	}
	return false;
}

UGUIS_GameUIContext* UGUIS_GameUIPolicy::FindContext(const ULocalPlayer* LocalPlayer, TSubclassOf<UGUIS_GameUIContext> ContextClass)
{
	if (FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		for (int32 i = 0; i < LayoutInfo->Contexts.Num(); i++)
		{
			if (LayoutInfo->Contexts[i] && LayoutInfo->Contexts[i]->GetClass() == ContextClass)
			{
				return LayoutInfo->Contexts[i];
			}
		}
	}
	return nullptr;
}

void UGUIS_GameUIPolicy::RemoveContext(const ULocalPlayer* LocalPlayer, TSubclassOf<UGUIS_GameUIContext> ContextClass)
{
	if (FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		int32 FoundContext = INDEX_NONE;
		for (int32 i = 0; i < LayoutInfo->Contexts.Num(); i++)
		{
			if (LayoutInfo->Contexts[i] && LayoutInfo->Contexts[i]->GetClass() == ContextClass)
			{
				FoundContext = i;
				UE_LOG(LogGUIS, Verbose, TEXT("[%s] unregistered context of type(%s) for %s."), *GetName(), *LayoutInfo->Contexts[i]->GetClass()->GetName(), *GetNameSafe(LocalPlayer));
				break;
			}
		}
		LayoutInfo->Contexts.RemoveAt(FoundContext);
	}
}

void UGUIS_GameUIPolicy::AddUIAction(const ULocalPlayer* LocalPlayer, UCommonUserWidget* Target, const FDataTableRowHandle& InputAction, bool bShouldDisplayInActionBar,
                                     const FGUIS_UIActionExecutedDelegate& Callback, FGUIS_UIActionBindingHandle& BindingHandle)
{
	if (FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		if (IsValid(Target))
		{
			FBindUIActionArgs BindArgs(InputAction, bShouldDisplayInActionBar, FSimpleDelegate::CreateLambda([InputAction, Callback]()
			{
				Callback.ExecuteIfBound(InputAction.RowName);
			}));
			BindingHandle.Handle = Target->RegisterUIActionBinding(BindArgs);
			LayoutInfo->BindingHandles.Add(BindingHandle.Handle);
		}
	}
}

void UGUIS_GameUIPolicy::RemoveUIAction(const ULocalPlayer* LocalPlayer, FGUIS_UIActionBindingHandle& BindingHandle)
{
	if (FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		if (BindingHandle.Handle.IsValid())
		{
			UE_LOG(LogGUIS, Display, TEXT("Unregister binding for %s"), *BindingHandle.Handle.GetDisplayName().ToString())

			BindingHandle.Handle.Unregister();
			LayoutInfo->BindingHandles.Remove(BindingHandle.Handle);
		}
	}
}

void UGUIS_GameUIPolicy::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);

	if (FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidget(LocalPlayer);
	}
}

void UGUIS_GameUIPolicy::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (FGUIS_RootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = false;

		LayoutInfo->Contexts.Empty();

		if (LocalMultiplayerInteractionMode == EGUIS_LocalMultiplayerInteractionMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
		{
			UGUIS_GameUILayout* RootLayout = LayoutInfo->RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				// We're removing a secondary player's root while it's in control - transfer control back to the primary player's root
				RootLayout->SetIsDormant(true);
				for (const FGUIS_RootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
				{
					if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
					{
						if (UGUIS_GameUILayout* PrimaryRootLayout = RootLayoutInfo.RootLayout)
						{
							PrimaryRootLayout->SetIsDormant(false);
						}
					}
				}
			}
		}
	}
}

void UGUIS_GameUIPolicy::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UGUIS_GameUILayout* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayout;
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemoveLayoutFromViewport(LocalPlayer, Layout);

		OnRootLayoutReleased(LocalPlayer, Layout);
	}
}

void UGUIS_GameUIPolicy::AddLayoutToViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout)
{
	UE_LOG(LogGUIS, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UGUIS_GameUIPolicy::RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogGUIS, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogGUIS, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."),
			       *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UGUIS_GameUIPolicy::OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// So our controller will work in PIE without needing to click in the viewport
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
	BP_OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UGUIS_GameUIPolicy::OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout)
{
	BP_OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
}

void UGUIS_GameUIPolicy::OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UGUIS_GameUILayout* Layout)
{
	BP_OnRootLayoutReleased(LocalPlayer, Layout);
}

void UGUIS_GameUIPolicy::RequestPrimaryControl(UGUIS_GameUILayout* Layout)
{
	if (LocalMultiplayerInteractionMode == EGUIS_LocalMultiplayerInteractionMode::SingleToggle && Layout->IsDormant())
	{
		for (const FGUIS_RootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UGUIS_GameUILayout* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}
}

void UGUIS_GameUIPolicy::CreateLayoutWidget(ULocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		TSubclassOf<UGUIS_GameUILayout> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UGUIS_GameUILayout* NewLayoutObject = CreateWidget<UGUIS_GameUILayout>(PlayerController, LayoutWidgetClass);
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);

			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

TSubclassOf<UGUIS_GameUILayout> UGUIS_GameUIPolicy::GetLayoutWidgetClass(ULocalPlayer* LocalPlayer)
{
	return LayoutClass.LoadSynchronous();
}
