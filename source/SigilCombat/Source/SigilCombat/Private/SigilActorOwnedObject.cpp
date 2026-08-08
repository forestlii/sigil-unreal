// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilActorOwnedObject.h"
#include "GameFramework/Actor.h"

UWorld* USigilActorOwnedObject::GetWorld() const
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
