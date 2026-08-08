// Copyright 2024 RedMoonGames All Rights Reserved.

#include "Utility/GMS_Tags.h"

namespace GMS_MovementModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(None, FName{TEXTVIEW("GMS.LocomotionMode.None")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName{TEXTVIEW("GMS.LocomotionMode.Grounded")})
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName{TEXTVIEW("GMS.LocomotionMode.InAir")})
	UE_DEFINE_GAMEPLAY_TAG(Flying, FName{TEXTVIEW("GMS.LocomotionMode.Flying")})
	UE_DEFINE_GAMEPLAY_TAG(Swimming, FName{TEXTVIEW("GMS.LocomotionMode.Swimming")})
}

namespace GMS_RotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, FName{TEXTVIEW("GMS.RotationMode.VelocityDirection")})
	UE_DEFINE_GAMEPLAY_TAG(ViewDirection, FName{TEXTVIEW("GMS.RotationMode.ViewDirection")})
}

namespace GMS_MovementStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Walk, FName{TEXTVIEW("GMS.MovementState.Walk")})
	UE_DEFINE_GAMEPLAY_TAG(Jog, FName{TEXTVIEW("GMS.MovementState.Jog")})
	UE_DEFINE_GAMEPLAY_TAG(Sprint, FName{TEXTVIEW("GMS.MovementState.Sprint")})
}

namespace GMS_OverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(None, FName{TEXTVIEW("GMS.OverlayMode.None")})
	UE_DEFINE_GAMEPLAY_TAG(Default, FName{TEXTVIEW("GMS.OverlayMode.Default")})
}

namespace GMS_SMTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Root, FName{TEXTVIEW("GMS.SM")}, "State Machine Root Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InAir, FName{TEXTVIEW("GMS.SM.InAir")}, "InAir States")
	UE_DEFINE_GAMEPLAY_TAG(InAir_Jump, FName{TEXTVIEW("GMS.SM.InAir.Jump")})
	UE_DEFINE_GAMEPLAY_TAG(InAir_Fall, FName{TEXTVIEW("GMS.SM.InAir.Fall")})
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Grounded, FName{TEXTVIEW("GMS.SM.Grounded")}, "Grounded States")
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Idle, FName{TEXTVIEW("GMS.SM.Grounded.Idle")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Start, FName{TEXTVIEW("GMS.SM.Grounded.Start")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Cycle, FName{TEXTVIEW("GMS.SM.Grounded.Cycle")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Stop, FName{TEXTVIEW("GMS.SM.Grounded.Stop")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Pivot, FName{TEXTVIEW("GMS.SM.Grounded.Pivot")})
	UE_DEFINE_GAMEPLAY_TAG(Grounded_Land, FName{TEXTVIEW("GMS.SM.Grounded.Land")})
}
