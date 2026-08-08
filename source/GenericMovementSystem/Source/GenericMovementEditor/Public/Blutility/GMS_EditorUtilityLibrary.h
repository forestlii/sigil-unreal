// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GMS_EditorUtilityLibrary.generated.h"

class UAnimationModifier;
/**
 * 
 */
UCLASS(Blueprintable)
class GENERICMOVEMENTEDITOR_API UGMS_EditorUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category="GMS|Development|Editor")
	static void RevertAnimModifierOfClass(TSubclassOf<UAnimationModifier> ModifierClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Development|Editor")
	static float GetSamplingFrameRate(const UAnimSequence* AnimSequence);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Development|Editor")
	static TArray<FName> GetAllCurveNames(const UAnimSequence* AnimSequence);
#endif
};
