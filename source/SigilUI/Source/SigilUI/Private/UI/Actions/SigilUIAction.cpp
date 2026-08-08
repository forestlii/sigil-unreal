// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/Actions/SigilUIAction.h"


USigilUIAction::USigilUIAction(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

bool USigilUIAction::IsCompatible(const UObject* Data) const
{
	return IsCompatibleInternal(Data);
}


bool USigilUIAction::CanInvokeInternal_Implementation(const UObject* Data, APlayerController* PlayerController) const
{
	// 与其他验证不同, 这个默认不通过, Override里修改
	return false;
}

bool USigilUIAction::CanInvoke(const UObject* Data, APlayerController* PlayerController) const
{
	return CanInvokeInternal(Data, PlayerController);
}

void USigilUIAction::InvokeAction(const UObject* Data, APlayerController* PlayerController) const
{
	if (CanInvoke(Data, PlayerController))
	{
		InvokeActionInternal(Data, PlayerController);
	}
}

FText USigilUIAction::GetActionName() const
{
	return DisplayName;
}

FName USigilUIAction::GetActionID() const
{
	return ActionID;
}

bool USigilUIAction::IsCompatibleInternal_Implementation(const UObject* Data) const
{
	return true;
}

void USigilUIAction::InvokeActionInternal_Implementation(const UObject* Data, APlayerController* PlayerController) const
{
}

UWorld* USigilUIAction::GetWorld() const
{
	if (UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}
	return nullptr;
}
