// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GCS_ActorOwnedObject.generated.h"

/**
 * Base class for objects owned by an actor.
 * 演员拥有的对象的基类。
 */
UCLASS(NotBlueprintable, Abstract)
class GENERICCOMBATSYSTEM_API UGCS_ActorOwnedObject : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Gets the world context for the object.
	 * 获取对象的世界上下文。
	 * @return The world context. 世界上下文。
	 */
	virtual UWorld* GetWorld() const override;
};