// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GCS_GameplayTags.h"

namespace GCS_BulletLaunch
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Always, "GGF.Combat.Bullet.LaunchCond.Always", "Always generate bullet");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DidNotHitPawn, "GGF.Combat.Bullet.LaunchCond.DidNotHitPawn", "Only generate bullet if not hit any pawn");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitPawn, "GGF.Combat.Bullet.LaunchCond.HitPawn", "Only generate bullet if hit any pawn");
}
