// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilCombatTags.h"

namespace SigilBulletLaunch
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Always, "Sigil.Combat.Bullet.LaunchCond.Always", "Always generate bullet");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DidNotHitPawn, "Sigil.Combat.Bullet.LaunchCond.DidNotHitPawn", "Only generate bullet if not hit any pawn");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitPawn, "Sigil.Combat.Bullet.LaunchCond.HitPawn", "Only generate bullet if hit any pawn");
}
