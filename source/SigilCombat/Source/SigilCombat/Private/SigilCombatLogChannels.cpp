// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilCombatLogChannels.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogSigilCombat)
DEFINE_LOG_CATEGORY(LogSigilCombat_Targeting)
DEFINE_LOG_CATEGORY(LogSigilCombat_Collision)

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}
