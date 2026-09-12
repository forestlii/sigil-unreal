// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SigilAbilitySourceInterface.generated.h"

/**
 * Interface for objects that can be used as the SourceObject of a granted ability and report whether they are
 * currently "active" (for example the weapon that is currently equipped). USigilGameplayAbility consults this
 * interface when bRequireSourceObjectActive is set.
 * 可作为技能 SourceObject 的对象接口，用于汇报自身当前是否"激活"（例如当前已装备的武器）。
 * USigilGameplayAbility 在 bRequireSourceObjectActive 开启时会查询该接口。
 */
UINTERFACE(Blueprintable, MinimalAPI)
class USigilAbilitySourceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementation class for the ability source interface.
 * 技能来源接口的实现类。
 */
class SIGILGAS_API ISigilAbilitySourceInterface
{
	GENERATED_BODY()

public:
	/**
	 * Returns true while abilities granted by this object are allowed to activate.
	 * 当由本对象授予的技能允许激活时返回 true。
	 * @return True if the source is active. 来源处于激活态则返回 true。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GGA|Ability|Source")
	bool IsAbilitySourceActive() const;
	virtual bool IsAbilitySourceActive_Implementation() const { return true; }
};
