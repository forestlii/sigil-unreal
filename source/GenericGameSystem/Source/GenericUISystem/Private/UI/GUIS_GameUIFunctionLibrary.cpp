// Copyright 2025 RedMoonGames All Rights Reserved.

#include "UI/GUIS_GameUIFunctionLibrary.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "GUIS_LogChannels.h"
#include "Components/ListView.h"
#include "Engine/GameInstance.h"
#include "UI/GUIS_GameUILayout.h"
#include "UI/GUIS_GameUIPolicy.h"
#include "UI/GUIS_GameUISubsystem.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIS_GameUIFunctionLibrary)

int32 UGUIS_GameUIFunctionLibrary::InputSuspensions = 0;

ECommonInputType UGUIS_GameUIFunctionLibrary::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType();
		}
	}

	return ECommonInputType::Count;
}

bool UGUIS_GameUIFunctionLibrary::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
		}
	}
	return false;
}

bool UGUIS_GameUIFunctionLibrary::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
		}
	}
	return false;
}

UCommonActivatableWidget* UGUIS_GameUIFunctionLibrary::PushContentToUILayer_ForPlayer(const APlayerController* PlayerController, FGameplayTag LayerName,
                                                                                      TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(PlayerController) || !ensure(WidgetClass != nullptr))
	{
		return nullptr;
	}

	UGUIS_GameUILayout* UILayout = GetGameUILayoutForPlayer(PlayerController);

	if (UILayout == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayer_ForPlayer failed to find UILayout for player."), ELogVerbosity::Error);
		return nullptr;
	}

	return UILayout->PushWidgetToLayerStack(LayerName, WidgetClass);
}

void UGUIS_GameUIFunctionLibrary::PopContentFromUILayer_ForPlayer(const APlayerController* PlayerController, FGameplayTag LayerName, int32 RemainNum)
{
	if (!ensure(PlayerController))
	{
		return;
	}

	UGUIS_GameUILayout* UILayout = GetGameUILayoutForPlayer(PlayerController);

	if (UILayout == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("PopContentFromUILayer_ForPlayer failed to find UILayout for player."), ELogVerbosity::Error);
		return;
	}

	if (UCommonActivatableWidgetContainerBase* Layer = UILayout->GetLayerWidget(LayerName))
	{
		const TArray<UCommonActivatableWidget*>& List = Layer->GetWidgetList();

		int32 MinIdx = RemainNum >= 1 ? RemainNum - 1 : 0;

		for (int32 i = List.Num() - 1; i >= MinIdx; i--)
		{
			Layer->RemoveWidget(*List[i]);
		}
	}
}

// void UGUIS_GameUIFunctionLibrary::PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
// {
// 	if (!ensure(LocalPlayer) || !ensure(!WidgetClass.IsNull()))
// 	{
// 		return;
// 	}
//
// 	if (UGameUIManagerSubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameUIManagerSubsystem>())
// 	{
// 		if (UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
// 		{
// 			if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer)))
// 			{
// 				const bool bSuspendInputUntilComplete = true;
// 				RootLayout->PushWidgetToLayerStackAsync(LayerName, bSuspendInputUntilComplete, WidgetClass);
// 			}
// 		}
// 	}
// }

void UGUIS_GameUIFunctionLibrary::PopContentFromUILayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget)
	{
		// Ignore request to pop an already deleted widget
		return;
	}

	if (const APlayerController* PlayerController = ActivatableWidget->GetOwningPlayer())
	{
		if (UGUIS_GameUILayout* UILayout = GetGameUILayoutForPlayer(PlayerController))
		{
			UE_LOG(LogGUIS, Verbose, TEXT("Popped content:%s from ui layer."), *GetNameSafe(ActivatableWidget))
			UILayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
		}
	}
}

void UGUIS_GameUIFunctionLibrary::PopContentsFromUILayer(TArray<UCommonActivatableWidget*> ActivatableWidgets, bool bReverse)
{
	if (bReverse)
	{
		for (int32 i = ActivatableWidgets.Num() - 1; i >= 0; i--)
		{
			PopContentFromUILayer(ActivatableWidgets[i]);
		}
	}
	else
	{
		for (int32 i = 0; i < ActivatableWidgets.Num(); i++)
		{
			PopContentFromUILayer(ActivatableWidgets[i]);
		}
	}
}

ULocalPlayer* UGUIS_GameUIFunctionLibrary::GetLocalPlayerFromController(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		return Cast<ULocalPlayer>(PlayerController->Player);
	}

	return nullptr;
}

UGUIS_GameUILayout* UGUIS_GameUIFunctionLibrary::GetGameUILayoutForPlayer(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->GetLocalPlayer()))
	{
		const ULocalPlayer* CommonLocalPlayer = CastChecked<ULocalPlayer>(LocalPlayer);
		if (const UGameInstance* GameInstance = CommonLocalPlayer->GetGameInstance())
		{
			if (UGUIS_GameUISubsystem* UIManager = GameInstance->GetSubsystem<UGUIS_GameUISubsystem>())
			{
				if (const UGUIS_GameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				{
					if (UGUIS_GameUILayout* RootLayout = Policy->GetRootLayout(CommonLocalPlayer))
					{
						return RootLayout;
					}
				}
			}
		}
	}
	return nullptr;
}

FName UGUIS_GameUIFunctionLibrary::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName UGUIS_GameUIFunctionLibrary::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensions++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensions);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UGUIS_GameUIFunctionLibrary::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UGUIS_GameUIFunctionLibrary::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None)
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}

UObject* UGUIS_GameUIFunctionLibrary::GetTypedListItem(TScriptInterface<IUserObjectListEntry> UserObjectListEntry, TSubclassOf<UObject> DesiredClass)
{
	UUserWidget* EntryWidget = Cast<UUserWidget>(UserObjectListEntry.GetObject());
	if (!IsValid(EntryWidget))
	{
		return nullptr;
	}
	UListView* OwningListView = Cast<UListView>(UUserListEntryLibrary::GetOwningListView(EntryWidget));
	if (!IsValid(OwningListView))
	{
		return nullptr;
	}

	UObject* ListItem = *OwningListView->ItemFromEntryWidget(*EntryWidget);

	if (ListItem->GetClass()->IsChildOf(DesiredClass))
	{
		return ListItem;
	}
	return nullptr;
}

bool UGUIS_GameUIFunctionLibrary::GetTypedListItemSafely(TScriptInterface<IUserObjectListEntry> UserObjectListEntry, TSubclassOf<UObject> DesiredClass, UObject*& OutItem)
{
	OutItem = GetTypedListItem(UserObjectListEntry, DesiredClass);
	return OutItem != nullptr;
}
