// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/SigilWeaponActor.h"
#include "SigilCombatTestTypes.generated.h"

/** Concrete weapon actor for automation tests (the base class is abstract). 自动化测试用的具体武器 Actor（基类是抽象的）。 */
UCLASS(Transient, NotPlaceable)
class ASigilCombatTestWeaponActor final : public ASigilWeaponActor
{
	GENERATED_BODY()
};
