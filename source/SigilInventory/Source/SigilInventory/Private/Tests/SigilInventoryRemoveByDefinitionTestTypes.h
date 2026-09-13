// Copyright (c) 2026 Likeon. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "SigilItemRestriction.h"
#include "SigilInventoryRemoveByDefinitionTestTypes.generated.h"

// 仅供自动化配置真实集合限制器，不替换库存或删除逻辑。
UCLASS(Transient)
class USigilRemovalTestRestriction final : public USigilItemRestriction
{
	GENERATED_BODY()
public:
	mutable int32 RemoveCalls = 0;
	int32 SuccessfulCalls = MAX_int32;
	int32 PerCallLimit = MAX_int32;

	virtual bool CanRemoveItemInternal_Implementation(FSigilItemInfo& Info) const override
	{
		++RemoveCalls;
		if (RemoveCalls > SuccessfulCalls) { return false; }
		Info.Amount = FMath::Min(Info.Amount, PerCallLimit);
		return true;
	}
};
