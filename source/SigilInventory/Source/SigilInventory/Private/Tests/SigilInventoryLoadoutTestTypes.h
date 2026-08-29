// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "SigilInventorySystemComponent.h"
#include "SigilInventoryLoadoutTestTypes.generated.h"

UCLASS()
class USigilInventoryLoadoutTestComponent final : public USigilInventorySystemComponent
{
	GENERATED_BODY()

public:
	int32 LocalLoadoutCallCount = 0;

	void InvokeServerLoadDefaultLoadoutsImplementation()
	{
		ServerLoadDefaultLoadouts_Implementation();
	}

	virtual void LoadDefaultLoadouts() override
	{
		++LocalLoadoutCallCount;
	}
};
