// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UI/SigilGameUISubsystem.h"
#include "GameFramework/Pawn.h"
#include "SigilUISettings.h"
#include "CommonUserWidget.h"
#include "SigilUILogChannels.h"
#include "Engine/GameInstance.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/SigilGameUIContext.h"
#include "UI/SigilGameUIPolicy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilGameUISubsystem)

class FSubsystemCollectionBase;
class UClass;

void USigilGameUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (USigilUISettings::Get()->GameUIPolicyClass.IsNull())
	{
		UE_LOG(LogSigilUI, Error, TEXT("SigilGameUISubsystem::Initialize Failed, Missing GameUIPolicyClass in SigilUISettings!!!"));
		return;
	}

	if (!CurrentPolicy)
	{
		TSubclassOf<USigilGameUIPolicy> PolicyClass = USigilUISettings::Get()->GameUIPolicyClass.LoadSynchronous();
		if (PolicyClass)
		{
			USigilGameUIPolicy* NewPolicy = NewObject<USigilGameUIPolicy>(this, PolicyClass);
			if (NewPolicy)
			{
				SwitchToPolicy(NewPolicy);
			}
			else
			{
				UE_LOG(LogSigilUI, Error, TEXT("SigilGameUISubsystem::Initialize Failed, failed to create Game UI Policy from class:%s!"), *PolicyClass->GetName());
			}
		}
		else
		{
			UE_LOG(LogSigilUI, Error, TEXT("SigilGameUISubsystem::Initialize Failed, Missing GameUIPolicyClass in SigilUISettings!!!"));
		}
	}
}

void USigilGameUISubsystem::Deinitialize()
{
	Super::Deinitialize();

	SwitchToPolicy(nullptr);
}

bool USigilGameUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		return false;
	}

	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	if (ChildClasses.Num() == 0)
	{
		UE_LOG(LogSigilUI, Display, TEXT("No override implementation found for USigilGameUISubsystem, So creating it."))
		return true;
	}
	return false;
}

void USigilGameUISubsystem::AddPlayer(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerAdded(LocalPlayer);
}

void USigilGameUISubsystem::RemovePlayer(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerDestroyed(LocalPlayer);
}

void USigilGameUISubsystem::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
	}
}

void USigilGameUISubsystem::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
	}
}

void USigilGameUISubsystem::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer);
	}
}

void USigilGameUISubsystem::RegisterUIActionBinding(UCommonUserWidget* Target, FDataTableRowHandle InputAction, bool bShouldDisplayInActionBar, const FSigilUIActionExecutedDelegate& Callback,
                                                    FSigilUIActionBindingHandle& BindingHandle)
{
	if (IsValid(Target))
	{
		FBindUIActionArgs BindArgs(InputAction, bShouldDisplayInActionBar, FSimpleDelegate::CreateLambda([InputAction, Callback]()
		{
			Callback.ExecuteIfBound(InputAction.RowName);
		}));

		BindingHandle.Handle = Target->RegisterUIActionBinding(BindArgs);
		BindingHandles.Add(BindingHandle.Handle);
	}
}

void USigilGameUISubsystem::UnregisterBinding(FSigilUIActionBindingHandle& BindingHandle)
{
	if (BindingHandle.Handle.IsValid())
	{
		UE_LOG(LogSigilUI, Display, TEXT("Unregister binding for %s"), *BindingHandle.Handle.GetDisplayName().ToString())

		BindingHandle.Handle.Unregister();
		BindingHandles.Remove(BindingHandle.Handle);
	}
}

void USigilGameUISubsystem::RegisterUIActionBindingForPlayer(ULocalPlayer* LocalPlayer, UCommonUserWidget* Target, FDataTableRowHandle InputAction, bool bShouldDisplayInActionBar,
                                                             const FSigilUIActionExecutedDelegate& Callback, FSigilUIActionBindingHandle& BindingHandle)
{
	if (LocalPlayer && CurrentPolicy)
	{
		CurrentPolicy->AddUIAction(LocalPlayer, Target, InputAction, bShouldDisplayInActionBar, Callback, BindingHandle);
	}
}

void USigilGameUISubsystem::UnregisterUIActionBindingForPlayer(ULocalPlayer* LocalPlayer, FSigilUIActionBindingHandle& BindingHandle)
{
	if (LocalPlayer && CurrentPolicy)
	{
		CurrentPolicy->RemoveUIAction(LocalPlayer, BindingHandle);
	}
}

void USigilGameUISubsystem::RegisterUIContextForPlayer(ULocalPlayer* LocalPlayer, USigilGameUIContext* Context, FSigilUIContextBindingHandle& BindingHandle)
{
	if (LocalPlayer && CurrentPolicy && Context)
	{
		if (CurrentPolicy->AddContext(LocalPlayer, Context))
		{
			BindingHandle = FSigilUIContextBindingHandle(LocalPlayer, Context->GetClass());
		}
	}
}

void USigilGameUISubsystem::RegisterUIContextForActor(AActor* Actor, USigilGameUIContext* Context, FSigilUIContextBindingHandle& BindingHandle)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogSigilUI, Error, TEXT("Trying to register ui context for invalid pawn!"))
		return;
	}
	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn == nullptr || !Pawn->IsLocallyControlled())
	{
		UE_LOG(LogSigilUI, Error, TEXT("Trying to register ui context for actor(%s) which is not locally controlled pawn!"), *Pawn->GetName())
		return;
	}
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (PC == nullptr)
	{
		UE_LOG(LogSigilUI, Error, TEXT("Trying to register ui context for pawn(%s) which doesn't have valid player controller"), *Pawn->GetName())
		return;
	}
	RegisterUIContextForPlayer(PC->GetLocalPlayer(), Context, BindingHandle);
}

bool USigilGameUISubsystem::FindUIContextForPlayer(ULocalPlayer* LocalPlayer, TSubclassOf<USigilGameUIContext> ContextClass, USigilGameUIContext*& OutContext)
{
	if (LocalPlayer && CurrentPolicy && ContextClass != nullptr)
	{
		if (USigilGameUIContext* Context = CurrentPolicy->GetContext(LocalPlayer, ContextClass))
		{
			if (Context->GetClass() == ContextClass)
			{
				OutContext = Context;
				return true;
			}
		}
	}
	return false;
}

bool USigilGameUISubsystem::FindUIContextFromHandle(FSigilUIContextBindingHandle& BindingHandle, TSubclassOf<USigilGameUIContext> ContextClass, USigilGameUIContext*& OutContext)
{
	if (BindingHandle.LocalPlayer && BindingHandle.ContextClass && CurrentPolicy)
	{
		OutContext = CurrentPolicy->FindContext(BindingHandle.LocalPlayer, BindingHandle.ContextClass);
	}
	return OutContext != nullptr;
}

void USigilGameUISubsystem::UnregisterUIContextForPlayer(FSigilUIContextBindingHandle& BindingHandle)
{
	if (BindingHandle.LocalPlayer && BindingHandle.ContextClass && CurrentPolicy)
	{
		CurrentPolicy->RemoveContext(BindingHandle.LocalPlayer, BindingHandle.ContextClass);
		BindingHandle.LocalPlayer = nullptr;
		BindingHandle.ContextClass = nullptr;
	}
}

void USigilGameUISubsystem::SwitchToPolicy(USigilGameUIPolicy* InPolicy)
{
	if (CurrentPolicy != InPolicy)
	{
		CurrentPolicy = InPolicy;
	}
}
