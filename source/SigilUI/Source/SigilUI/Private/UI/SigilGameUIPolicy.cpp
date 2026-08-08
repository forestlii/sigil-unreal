// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/SigilGameUIPolicy.h"
#include "UI/SigilGameUISubsystem.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "SigilUILogChannels.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/SigilGameUIContext.h"
#include "UI/SigilGameUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilGameUIPolicy)

// Static
USigilGameUIPolicy* USigilGameUIPolicy::GetGameUIPolicy(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (USigilGameUISubsystem* UIManager = UGameInstance::GetSubsystem<USigilGameUISubsystem>(GameInstance))
			{
				return UIManager->GetCurrentUIPolicy();
			}
		}
	}

	return nullptr;
}

USigilGameUISubsystem* USigilGameUIPolicy::GetOwningSubsystem() const
{
	return Cast<USigilGameUISubsystem>(GetOuter());
}

UWorld* USigilGameUIPolicy::GetWorld() const
{
	if (USigilGameUISubsystem* Subsystem = GetOwningSubsystem())
	{
		return Subsystem->GetGameInstance()->GetWorld();
	}
	return nullptr;
}

USigilGameUILayout* USigilGameUIPolicy::GetRootLayout(const ULocalPlayer* LocalPlayer) const
{
	const FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}

USigilGameUIContext* USigilGameUIPolicy::GetContext(const ULocalPlayer* LocalPlayer, TSubclassOf<USigilGameUIContext> ContextClass)
{
	if (const FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
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

bool USigilGameUIPolicy::AddContext(const ULocalPlayer* LocalPlayer, USigilGameUIContext* NewContext)
{
	if (FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		if (const UObject* ExistingContext = GetContext(LocalPlayer, NewContext->GetClass()))
		{
			UE_LOG(LogSigilUI, Warning, TEXT("[%s] is trying to add repeat context of type(%s) for %s, which is not allowed!"), *GetName(), *NewContext->GetClass()->GetName(), *GetNameSafe(LocalPlayer));
			return false;
		}
		LayoutInfo->Contexts.Add(NewContext);
		UE_LOG(LogSigilUI, Verbose, TEXT("[%s] registered context of type(%s) for %s."), *GetName(), *NewContext->GetClass()->GetName(), *GetNameSafe(LocalPlayer));
		return true;
	}
	return false;
}

USigilGameUIContext* USigilGameUIPolicy::FindContext(const ULocalPlayer* LocalPlayer, TSubclassOf<USigilGameUIContext> ContextClass)
{
	if (FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
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

void USigilGameUIPolicy::RemoveContext(const ULocalPlayer* LocalPlayer, TSubclassOf<USigilGameUIContext> ContextClass)
{
	if (FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		int32 FoundContext = INDEX_NONE;
		for (int32 i = 0; i < LayoutInfo->Contexts.Num(); i++)
		{
			if (LayoutInfo->Contexts[i] && LayoutInfo->Contexts[i]->GetClass() == ContextClass)
			{
				FoundContext = i;
				UE_LOG(LogSigilUI, Verbose, TEXT("[%s] unregistered context of type(%s) for %s."), *GetName(), *LayoutInfo->Contexts[i]->GetClass()->GetName(), *GetNameSafe(LocalPlayer));
				break;
			}
		}

		if (FoundContext != INDEX_NONE)
		{
			LayoutInfo->Contexts.RemoveAt(FoundContext);
		}
	}
}

void USigilGameUIPolicy::AddUIAction(const ULocalPlayer* LocalPlayer, UCommonUserWidget* Target, const FDataTableRowHandle& InputAction, bool bShouldDisplayInActionBar,
                                     const FSigilUIActionExecutedDelegate& Callback, FSigilUIActionBindingHandle& BindingHandle)
{
	if (FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
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

void USigilGameUIPolicy::RemoveUIAction(const ULocalPlayer* LocalPlayer, FSigilUIActionBindingHandle& BindingHandle)
{
	if (FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		if (BindingHandle.Handle.IsValid())
		{
			UE_LOG(LogSigilUI, Display, TEXT("Unregister binding for %s"), *BindingHandle.Handle.GetDisplayName().ToString())

			BindingHandle.Handle.Unregister();
			LayoutInfo->BindingHandles.Remove(BindingHandle.Handle);
		}
	}
}

void USigilGameUIPolicy::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);

	if (FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidget(LocalPlayer);
	}
}

void USigilGameUIPolicy::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (FSigilRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = false;

		LayoutInfo->Contexts.Empty();

		if (LocalMultiplayerInteractionMode == ESigilLocalMultiplayerInteractionMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
		{
			USigilGameUILayout* RootLayout = LayoutInfo->RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				// We're removing a secondary player's root while it's in control - transfer control back to the primary player's root
				RootLayout->SetIsDormant(true);
				for (const FSigilRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
				{
					if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
					{
						if (USigilGameUILayout* PrimaryRootLayout = RootLayoutInfo.RootLayout)
						{
							PrimaryRootLayout->SetIsDormant(false);
						}
					}
				}
			}
		}
	}
}

void USigilGameUIPolicy::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		USigilGameUILayout* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayout;
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemoveLayoutFromViewport(LocalPlayer, Layout);

		OnRootLayoutReleased(LocalPlayer, Layout);
	}
}

void USigilGameUIPolicy::AddLayoutToViewport(ULocalPlayer* LocalPlayer, USigilGameUILayout* Layout)
{
	UE_LOG(LogSigilUI, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void USigilGameUIPolicy::RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, USigilGameUILayout* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogSigilUI, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogSigilUI, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."),
			       *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void USigilGameUIPolicy::OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, USigilGameUILayout* Layout)
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

void USigilGameUIPolicy::OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, USigilGameUILayout* Layout)
{
	BP_OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
}

void USigilGameUIPolicy::OnRootLayoutReleased(ULocalPlayer* LocalPlayer, USigilGameUILayout* Layout)
{
	BP_OnRootLayoutReleased(LocalPlayer, Layout);
}

void USigilGameUIPolicy::RequestPrimaryControl(USigilGameUILayout* Layout)
{
	if (LocalMultiplayerInteractionMode == ESigilLocalMultiplayerInteractionMode::SingleToggle && Layout->IsDormant())
	{
		for (const FSigilRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			USigilGameUILayout* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}
}

void USigilGameUIPolicy::CreateLayoutWidget(ULocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		TSubclassOf<USigilGameUILayout> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			USigilGameUILayout* NewLayoutObject = CreateWidget<USigilGameUILayout>(PlayerController, LayoutWidgetClass);
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);

			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

TSubclassOf<USigilGameUILayout> USigilGameUIPolicy::GetLayoutWidgetClass(ULocalPlayer* LocalPlayer)
{
	return LayoutClass.LoadSynchronous();
}
