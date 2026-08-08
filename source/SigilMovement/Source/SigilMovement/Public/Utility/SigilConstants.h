// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilConstants.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class SIGILMOVEMENT_API USigilConstants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static const FName& TurnInPlaceSlotName();

	// Other Animation Curves

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawSpeedCurveName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawOffsetCurveName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowTurnInPlaceCurveName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowAimingCurveName();
};

inline const FName& USigilConstants::TurnInPlaceSlotName()
{
	static const FName Name{TEXTVIEW("TurnInPlace")};
	return Name;
}

inline const FName& USigilConstants::RotationYawSpeedCurveName()
{
	static const FName Name{TEXTVIEW("RotationYawSpeed")};
	return Name;
}

inline const FName& USigilConstants::RotationYawOffsetCurveName()
{
	static const FName Name{TEXTVIEW("RotationYawOffset")};
	return Name;
}

inline const FName& USigilConstants::AllowTurnInPlaceCurveName()
{
	static const FName Name{TEXTVIEW("AllowTurnInPlace")};
	return Name;
}

inline const FName& USigilConstants::AllowAimingCurveName()
{
	static const FName Name{TEXTVIEW("AllowAiming")};
	return Name;
}
