// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Locomotions/SigilLocomotionStructLibrary.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSequence.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilLocomotionStructLibrary)

bool FSigilAnimations_4Direction::ValidAnimations() const
{
	if (Forward && Forward->HasRootMotion())
	{
	}

	return Forward && Backward && Left && Right;
}

bool FSigilAnimations_4Direction::HasRootMotion() const
{
	if (ValidAnimations())
	{
		return Forward->HasRootMotion() && Backward->HasRootMotion() && Left->HasRootMotion() && Right->HasRootMotion();
	}
	return false;
}

bool FSigilAnimations_8Direction::ValidAnimations() const
{
	return Forward && ForwardLeft && ForwardRight && Backward && BackwardLeft && BackwardRight && Left && Right;
}

bool FSigilAnimations_8Direction::HasRootMotion() const
{
	if (ValidAnimations())
	{
		return Forward->HasRootMotion() && ForwardLeft->HasRootMotion() && ForwardRight->HasRootMotion() && Backward->HasRootMotion() && BackwardLeft->HasRootMotion() && BackwardRight->
			HasRootMotion() && Left->
			HasRootMotion() && Right->HasRootMotion();
	}
	return false;
}
