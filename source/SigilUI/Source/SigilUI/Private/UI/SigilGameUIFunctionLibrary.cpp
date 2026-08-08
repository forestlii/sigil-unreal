// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/SigilGameUIFunctionLibrary.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "SigilUILogChannels.h"
#include "Components/ListView.h"
#include "Engine/GameInstance.h"
#include "UI/SigilGameUILayout.h"
#include "UI/SigilGameUIPolicy.h"
#include "UI/SigilGameUISubsystem.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilGameUIFunctionLibrary)

int32 USigilGameUIFunctionLibrary::InputSuspensions = 0;

ECommonInputType USigilGameUIFunctionLibrary::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
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

bool USigilGameUIFunctionLibrary::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
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

bool USigilGameUIFunctionLibrary::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
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

UCommonActivatableWidget* USigilGameUIFunctionLibrary::PushContentToUILayer_ForPlayer(const APlayerController* PlayerController, FGameplayTag LayerName,
                                                                                      TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(PlayerController) || !ensure(WidgetClass != nullptr))
	{
		return nullptr;
	}

	USigilGameUILayout* UILayout = GetGameUILayoutForPlayer(PlayerController);

	if (UILayout == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToUILayer_ForPlayer failed to find UILayout for player."), ELogVerbosity::Error);
		return nullptr;
	}

	return UILayout->PushWidgetToLayerStack(LayerName, WidgetClass);
}

void USigilGameUIFunctionLibrary::PopContentFromUILayer_ForPlayer(const APlayerController* PlayerController, FGameplayTag LayerName, int32 RemainNum)
{
	if (!ensure(PlayerController))
	{
		return;
	}

	USigilGameUILayout* UILayout = GetGameUILayoutForPlayer(PlayerController);

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

void USigilGameUIFunctionLibrary::PopContentFromUILayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget)
	{
		// Ignore request to pop an already deleted widget
		return;
	}

	if (const APlayerController* PlayerController = ActivatableWidget->GetOwningPlayer())
	{
		if (USigilGameUILayout* UILayout = GetGameUILayoutForPlayer(PlayerController))
		{
			UE_LOG(LogSigilUI, Verbose, TEXT("Popped content:%s from ui layer."), *GetNameSafe(ActivatableWidget))
			UILayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
		}
	}
}

void USigilGameUIFunctionLibrary::PopContentsFromUILayer(TArray<UCommonActivatableWidget*> ActivatableWidgets, bool bReverse)
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

ULocalPlayer* USigilGameUIFunctionLibrary::GetLocalPlayerFromController(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		return Cast<ULocalPlayer>(PlayerController->Player);
	}

	return nullptr;
}

USigilGameUILayout* USigilGameUIFunctionLibrary::GetGameUILayoutForPlayer(const APlayerController* PlayerController)
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
			if (USigilGameUISubsystem* UIManager = GameInstance->GetSubsystem<USigilGameUISubsystem>())
			{
				if (const USigilGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				{
					if (USigilGameUILayout* RootLayout = Policy->GetRootLayout(CommonLocalPlayer))
					{
						return RootLayout;
					}
				}
			}
		}
	}
	return nullptr;
}

FName USigilGameUIFunctionLibrary::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName USigilGameUIFunctionLibrary::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
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

void USigilGameUIFunctionLibrary::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void USigilGameUIFunctionLibrary::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
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

UObject* USigilGameUIFunctionLibrary::GetTypedListItem(TScriptInterface<IUserObjectListEntry> UserObjectListEntry, TSubclassOf<UObject> DesiredClass)
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

bool USigilGameUIFunctionLibrary::GetTypedListItemSafely(TScriptInterface<IUserObjectListEntry> UserObjectListEntry, TSubclassOf<UObject> DesiredClass, UObject*& OutItem)
{
	OutItem = GetTypedListItem(UserObjectListEntry, DesiredClass);
	return OutItem != nullptr;
}
