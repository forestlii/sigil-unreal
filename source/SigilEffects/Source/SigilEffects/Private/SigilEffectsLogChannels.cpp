// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilEffectsLogChannels.h"
#include "UObject/Object.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

DEFINE_LOG_CATEGORY(LogSigilEffects)


FString GetGESLogContextString(const UObject* ContextObject)
{
	ENetRole Role = ROLE_None;
	FString RoleName = TEXT("None");
	FString Name = "None";

	if (const AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
		Name = Actor->GetName();
	}
	else if (const UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		if (AActor* ActorOwner = Cast<AActor>(Component->GetOuter()))
		{
			Role = ActorOwner->GetLocalRole();
			Name = ActorOwner->GetName();
		}
		else
		{
			const AActor* Owner = Component->GetOwner();
			Role = IsValid(Owner) ? Owner->GetLocalRole() : ROLE_None;
			Name = IsValid(Owner) ? Owner->GetName() : TEXT("None");
		}
	}
	else if (IsValid(ContextObject))
	{
		Name = ContextObject->GetName();
	}

	if (Role != ROLE_None)
	{
		RoleName = (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	return FString::Printf(TEXT("[%s] (%s)"), *RoleName, *Name);
}
