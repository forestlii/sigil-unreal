// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilInteractionLogChannels.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "GameplayBehavior.h"
#include "GameplayTask.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTask.h"

DEFINE_LOG_CATEGORY(LogSigilInteraction)

FString GetGGSLogContextString(const UObject* ContextObject)
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
		Role = Component->GetOwnerRole();
		Name = Component->GetOwner()->GetName();
	}
	else if (const UGameplayBehavior* Behavior = Cast<UGameplayBehavior>(ContextObject))
	{
		Role = Behavior->GetAvatar()->GetLocalRole();
		Name = Behavior->GetAvatar()->GetName();
	}
	else if (const UGameplayTask* Task = Cast<UGameplayTask>(ContextObject))
	{
		Role = Task->GetAvatarActor()->GetLocalRole();
		Name = Task->GetAvatarActor()->GetName();
	}
	else if (const UGameplayAbility* Ability = Cast<UGameplayAbility>(ContextObject))
	{
		Role = Ability->GetAvatarActorFromActorInfo()->GetLocalRole();
		Name = Ability->GetAvatarActorFromActorInfo()->GetName();
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
