// Copyright 2025 RedMoonGames All Rights Reserved.


#include "UI/GUIS_GameUIStructLibrary.h"
#include "Engine/LocalPlayer.h"
#include "UI/GUIS_GameUIContext.h"
#include "UI/GUIS_GameUILayout.h"

FGUIS_UIContextBindingHandle::FGUIS_UIContextBindingHandle(ULocalPlayer* InLocalPlayer, UClass* InContextClass)
{
	LocalPlayer = InLocalPlayer;
	ContextClass = InContextClass;
}
