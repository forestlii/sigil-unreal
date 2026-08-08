// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SigilEditorUtilityLibrary.generated.h"

class UAnimationModifier;
/**
 * 
 */
UCLASS(Blueprintable)
class SIGILMOVEMENTEDITOR_API USigilEditorUtilityLibrary : public UBlueprintFunctionLibrary
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
