// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GCS_LogChannels.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogGCS)
DEFINE_LOG_CATEGORY(LogGCS_Targeting)
DEFINE_LOG_CATEGORY(LogGCS_Collision)

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
