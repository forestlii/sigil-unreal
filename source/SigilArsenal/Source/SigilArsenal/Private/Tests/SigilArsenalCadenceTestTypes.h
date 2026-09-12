// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/SigilGameplayAbility_FireCadence.h"
#include "SigilArsenalCadenceTestTypes.generated.h"

/** 以真实节奏基类调度可计数的单发，并可在指定成功次数后拒绝下一发。 */
UCLASS(Transient)
class USigilArsenalTestCountingCadence : public USigilGameplayAbility_FireCadence
{
	GENERATED_BODY()

public:
	int32 AttemptCount = 0;
	int32 SuccessfulShotCount = 0;
	int32 MaxSuccessfulShots = INDEX_NONE;

	// 复现引擎在能力作用域锁内延后执行 EndAbility 的真实路径。
	void BeginScopeLockForTest() { IncrementListLock(); }
	void EndScopeLockForTest() { DecrementListLock(); }

protected:
	virtual bool TryFireOnce_Implementation() override;
};

/** 直接使用生产基类的单发查找与激活路径，仅将抽象基类具体化。 */
UCLASS(Transient)
class USigilArsenalTestRoutedCadence final : public USigilGameplayAbility_FireCadence
{
	GENERATED_BODY()
};

/** 在一次单发回调中取消自身并重启，用于检验旧回调不会结束或覆盖新一轮。 */
UCLASS(Transient)
class USigilArsenalTestReentrantCadence final : public USigilArsenalTestCountingCadence
{
	GENERATED_BODY()

public:
	bool bRestartOnNextShot = false;
	bool bRestartSucceeded = false;
	int32 RestartCount = 0;

protected:
	virtual bool TryFireOnce_Implementation() override;
};
