// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSigilCombat, Log, All)

DECLARE_LOG_CATEGORY_EXTERN(LogSigilCombat_Targeting, Log, All)

DECLARE_LOG_CATEGORY_EXTERN(LogSigilCombat_Collision, Log, All)


SIGILCOMBAT_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

#define SIGIL_COMBAT_VLOG(Verbosity, Format, ...) UE_VLOG(GetOwner(), LogGAIS_Command, Verbosity, Format, ##__VA_ARGS__)


