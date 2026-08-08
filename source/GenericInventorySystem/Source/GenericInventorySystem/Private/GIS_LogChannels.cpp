// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_LogChannels.h"
#include "UObject/Object.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "GIS_ItemInstance.h"
#include "GIS_ItemCollection.h"
#include "GIS_ItemDefinition.h"

DEFINE_LOG_CATEGORY(LogGIS)


FString GetGISLogContextString(const UObject* ContextObject)
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
	else if (const UGIS_ItemInstance* ItemInstance = Cast<UGIS_ItemInstance>(ContextObject))
	{
		if (AActor* ActorOwner = Cast<AActor>(ItemInstance->GetOuter()))
		{
			Role = ActorOwner->GetLocalRole();
			Name = ActorOwner->GetName();
		}
		else
		{
			return FString::Printf(TEXT("(%s)'s instance(%s) "), *ItemInstance->GetDefinition()->GetName(), *ItemInstance->GetName());
		}
	}
	else if (const UGIS_ItemCollection* Collection = Cast<UGIS_ItemCollection>(ContextObject))
	{
		if (AActor* ActorOwner = Cast<AActor>(Collection->GetOuter()))
		{
			Role = ActorOwner->GetLocalRole();
			Name = ActorOwner->GetName();
		}
		if (Role != ROLE_None)
		{
			RoleName = (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
		}
		return FString::Printf(TEXT("[%s] (%s)'s %s"), *RoleName, *Name, *Collection->GetCollectionName());
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
