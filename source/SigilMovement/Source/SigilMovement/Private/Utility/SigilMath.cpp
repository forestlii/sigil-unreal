// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utility/SigilMath.h"

#include "Locomotions/SigilLocomotionEnumLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilMath)

FVector USigilMath::VInterpEaseInOut(FVector A, FVector B, double Alpha, double Exponent)
{
	return FMath::InterpEaseInOut(A,B,Alpha,Exponent);
}

FVector USigilMath::SlerpSkipNormalization(const FVector& From, const FVector& To, const float Alpha)
{
	// http://allenchou.net/2018/05/game-math-deriving-the-slerp-formula/

	const auto Dot{From | To};

	if (Dot > 0.9995f || Dot < -0.9995f)
	{
		return FMath::Lerp(From, To, Alpha).GetSafeNormal();
	}

	const auto Theta{UE_REAL_TO_FLOAT(FMath::Acos(Dot)) * Alpha};

	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Theta);

	const auto FromPerpendicular{(To - From * Dot).GetSafeNormal()};

	return From * Cos + FromPerpendicular * Sin;
}

ESigilMovementDirection USigilMath::CalculateMovementDirection(const float Angle, const float ForwardHalfAngle, const float AngleThreshold)
{
	if (Angle >= -ForwardHalfAngle - AngleThreshold && Angle <= ForwardHalfAngle + AngleThreshold)
	{
		return ESigilMovementDirection::Forward;
	}

	if (Angle >= ForwardHalfAngle - AngleThreshold && Angle <= 180.0f - ForwardHalfAngle + AngleThreshold)
	{
		return ESigilMovementDirection::Right;
	}

	if (Angle <= -(ForwardHalfAngle - AngleThreshold) && Angle >= -(180.0f - ForwardHalfAngle + AngleThreshold))
	{
		return ESigilMovementDirection::Left;
	}

	return ESigilMovementDirection::Backward;
}

bool USigilMath::TryCalculatePoleVector(const FVector& ALocation, const FVector& BLocation, const FVector& CLocation,
                                      FVector& ProjectionLocation, FVector& Direction)
{
	const auto AbVector{BLocation - ALocation};
	if (AbVector.IsNearlyZero())
	{
		// Can't do anything if A and B are equal.

		ProjectionLocation = ALocation;
		Direction = FVector::ZeroVector;

		return false;
	}

	auto AcVector{CLocation - ALocation};
	if (!AcVector.Normalize())
	{
		// Only A and C are equal.

		ProjectionLocation = ALocation;
		Direction = AbVector.GetUnsafeNormal(); // A and B are not equal, so normalization will be safe.

		return true;
	}

	ProjectionLocation = ALocation + AbVector.ProjectOnToNormal(AcVector);
	Direction = BLocation - ProjectionLocation;

	return Direction.Normalize(); // Direction will be zero and cannot be normalized if A, B and C are collinear.
}
