// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "Utility/SigilMovementTags.h"

namespace SigilMovementModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(None, FName{TEXTVIEW("Sigil.Movement.LocomotionMode.None")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName{TEXTVIEW("Sigil.Movement.LocomotionMode.Grounded")})
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName{TEXTVIEW("Sigil.Movement.LocomotionMode.InAir")})
	UE_DEFINE_GAMEPLAY_TAG(Flying, FName{TEXTVIEW("Sigil.Movement.LocomotionMode.Flying")})
	UE_DEFINE_GAMEPLAY_TAG(Swimming, FName{TEXTVIEW("Sigil.Movement.LocomotionMode.Swimming")})
}

namespace SigilRotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, FName{TEXTVIEW("Sigil.Movement.RotationMode.VelocityDirection")})
	UE_DEFINE_GAMEPLAY_TAG(ViewDirection, FName{TEXTVIEW("Sigil.Movement.RotationMode.ViewDirection")})
}

namespace SigilMovementStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Walk, FName{TEXTVIEW("Sigil.Movement.State.Walk")})
	UE_DEFINE_GAMEPLAY_TAG(Jog, FName{TEXTVIEW("Sigil.Movement.State.Jog")})
	UE_DEFINE_GAMEPLAY_TAG(Sprint, FName{TEXTVIEW("Sigil.Movement.State.Sprint")})
}

namespace SigilOverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(None, FName{TEXTVIEW("Sigil.Movement.OverlayMode.None")})
	UE_DEFINE_GAMEPLAY_TAG(Default, FName{TEXTVIEW("Sigil.Movement.OverlayMode.Default")})
}

namespace SigilSMTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Root, FName{TEXTVIEW("Sigil.Movement.SM")}, "State Machine Root Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InAir, FName{TEXTVIEW("Sigil.Movement.SM.InAir")}, "InAir States")
	UE_DEFINE_GAMEPLAY_TAG(InAir_Jump, FName{TEXTVIEW("Sigil.Movement.SM.InAir.Jump")})
	UE_DEFINE_GAMEPLAY_TAG(InAir_Fall, FName{TEXTVIEW("Sigil.Movement.SM.InAir.Fall")})
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Grounded, FName{TEXTVIEW("Sigil.Movement.SM.Grounded")}, "Grounded States")
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Idle, FName{TEXTVIEW("Sigil.Movement.SM.Grounded.Idle")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Start, FName{TEXTVIEW("Sigil.Movement.SM.Grounded.Start")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Cycle, FName{TEXTVIEW("Sigil.Movement.SM.Grounded.Cycle")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Stop, FName{TEXTVIEW("Sigil.Movement.SM.Grounded.Stop")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Pivot, FName{TEXTVIEW("Sigil.Movement.SM.Grounded.Pivot")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Land, FName{TEXTVIEW("Sigil.Movement.SM.Grounded.Land")})
}
