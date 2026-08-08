// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "UI/SigilGameUIStructLibrary.h"
#include "Engine/LocalPlayer.h"
#include "UI/SigilGameUIContext.h"
#include "UI/SigilGameUILayout.h"

FSigilUIContextBindingHandle::FSigilUIContextBindingHandle(ULocalPlayer* InLocalPlayer, UClass* InContextClass)
{
	LocalPlayer = InLocalPlayer;
	ContextClass = InContextClass;
}
