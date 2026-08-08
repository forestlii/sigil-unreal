// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGCS, Log, All)

DECLARE_LOG_CATEGORY_EXTERN(LogGCS_Targeting, Log, All)

DECLARE_LOG_CATEGORY_EXTERN(LogGCS_Collision, Log, All)


GENERICCOMBATSYSTEM_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

#define GCS_VLOG(Verbosity, Format, ...) UE_VLOG(GetOwner(), LogGAIS_Command, Verbosity, Format, ##__VA_ARGS__)


