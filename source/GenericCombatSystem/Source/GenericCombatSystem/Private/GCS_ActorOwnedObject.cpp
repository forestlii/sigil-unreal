// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GCS_ActorOwnedObject.h"
#include "GameFramework/Actor.h"

UWorld* UGCS_ActorOwnedObject::GetWorld() const
{
	// To Make sure the outer is Valid and can be used
	if (!HasAnyFlags(RF_ClassDefaultObject) && !GetOuter()->HasAnyFlags(RF_BeginDestroyed) && !GetOuter()->IsUnreachable())
	{
		//Attempt to get the world 
		AActor* Outer = GetTypedOuter<AActor>();
		if (Outer != nullptr)
		{
			return Outer->GetWorld();
		}
	}
	return nullptr;
}
