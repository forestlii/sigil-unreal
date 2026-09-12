// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Tests/SigilArsenalCadenceTestTypes.h"

#include "AbilitySystemComponent.h"

bool USigilArsenalTestCountingCadence::TryFireOnce_Implementation()
{
	++AttemptCount;
	if (MaxSuccessfulShots != INDEX_NONE && SuccessfulShotCount >= MaxSuccessfulShots)
	{
		return false;
	}
	++SuccessfulShotCount;
	return true;
}

bool USigilArsenalTestReentrantCadence::TryFireOnce_Implementation()
{
	const bool bShotSucceeded = Super::TryFireOnce_Implementation();
	if (bRestartOnNextShot)
	{
		bRestartOnNextShot = false;
		++RestartCount;
		const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->CancelAbilityHandle(Handle);
			bRestartSucceeded = ASC->TryActivateAbility(Handle);
		}
		// 旧一轮返回失败，新一轮仍应按自己的激活序号继续运行。
		return false;
	}
	return bShotSucceeded;
}
