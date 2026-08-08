// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/Actions/GUIS_UIAction.h"


UGUIS_UIAction::UGUIS_UIAction(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

bool UGUIS_UIAction::IsCompatible(const UObject* Data) const
{
	return IsCompatibleInternal(Data);
}


bool UGUIS_UIAction::CanInvokeInternal_Implementation(const UObject* Data, APlayerController* PlayerController) const
{
	// 与其他验证不同, 这个默认不通过, Override里修改
	return false;
}

bool UGUIS_UIAction::CanInvoke(const UObject* Data, APlayerController* PlayerController) const
{
	return CanInvokeInternal(Data, PlayerController);
}

void UGUIS_UIAction::InvokeAction(const UObject* Data, APlayerController* PlayerController) const
{
	// if (CanInvoke(Data, User))
	{
		InvokeActionInternal(Data, PlayerController);
	}
}

FText UGUIS_UIAction::GetActionName() const
{
	return DisplayName;
}

FName UGUIS_UIAction::GetActionID() const
{
	return ActionID;
}

bool UGUIS_UIAction::IsCompatibleInternal_Implementation(const UObject* Data) const
{
	return true;
}

void UGUIS_UIAction::InvokeActionInternal_Implementation(const UObject* Data, APlayerController* PlayerController) const
{
}

UWorld* UGUIS_UIAction::GetWorld() const
{
	if (UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}
	return nullptr;
}
