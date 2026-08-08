// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Locomotions/GMS_LocomotionStructLibrary.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSequence.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_LocomotionStructLibrary)

bool FGMS_Animations_4Direction::ValidAnimations() const
{
	if (Forward && Forward->HasRootMotion())
	{
	}

	return Forward && Backward && Left && Right;
}

bool FGMS_Animations_4Direction::HasRootMotion() const
{
	if (ValidAnimations())
	{
		return Forward->HasRootMotion() && Backward->HasRootMotion() && Left->HasRootMotion() && Right->HasRootMotion();
	}
	return false;
}

bool FGMS_Animations_8Direction::ValidAnimations() const
{
	return Forward && ForwardLeft && ForwardRight && Backward && BackwardLeft && BackwardRight && Left && Right;
}

bool FGMS_Animations_8Direction::HasRootMotion() const
{
	if (ValidAnimations())
	{
		return Forward->HasRootMotion() && ForwardLeft->HasRootMotion() && ForwardRight->HasRootMotion() && Backward->HasRootMotion() && BackwardLeft->HasRootMotion() && BackwardRight->
			HasRootMotion() && Left->
			HasRootMotion() && Right->HasRootMotion();
	}
	return false;
}
