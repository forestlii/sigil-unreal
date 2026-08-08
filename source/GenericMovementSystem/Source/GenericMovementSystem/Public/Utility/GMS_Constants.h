// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GMS_Constants.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class GENERICMOVEMENTSYSTEM_API UGMS_Constants : public UBlueprintFunctionLibrary
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

inline const FName& UGMS_Constants::TurnInPlaceSlotName()
{
	static const FName Name{TEXTVIEW("TurnInPlace")};
	return Name;
}

inline const FName& UGMS_Constants::RotationYawSpeedCurveName()
{
	static const FName Name{TEXTVIEW("RotationYawSpeed")};
	return Name;
}

inline const FName& UGMS_Constants::RotationYawOffsetCurveName()
{
	static const FName Name{TEXTVIEW("RotationYawOffset")};
	return Name;
}

inline const FName& UGMS_Constants::AllowTurnInPlaceCurveName()
{
	static const FName Name{TEXTVIEW("AllowTurnInPlace")};
	return Name;
}

inline const FName& UGMS_Constants::AllowAimingCurveName()
{
	static const FName Name{TEXTVIEW("AllowAiming")};
	return Name;
}
