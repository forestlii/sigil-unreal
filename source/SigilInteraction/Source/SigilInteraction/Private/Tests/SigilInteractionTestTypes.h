// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/SigilInteractionSystemComponent.h"
#include "SigilInteractionTestTypes.generated.h"

UCLASS()
class USigilInteractionRefreshProbeComponent final : public USigilInteractionSystemComponent
{
	GENERATED_BODY()

public:
	int32 RefreshCallCount = 0;

protected:
	virtual void RefreshOptionsForActor() override
	{
		++RefreshCallCount;
	}
};
