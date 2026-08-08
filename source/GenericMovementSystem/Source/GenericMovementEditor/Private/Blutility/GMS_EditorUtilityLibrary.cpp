// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Blutility/GMS_EditorUtilityLibrary.h"

#include "AnimationModifier.h"
#include "AnimationModifiersAssetUserData.h"
#include "EditorUtilityLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_EditorUtilityLibrary)

void UGMS_EditorUtilityLibrary::RevertAnimModifierOfClass(TSubclassOf<UAnimationModifier> ModifierClass)
{
	if (!IsValid(ModifierClass))
	{
		return;
	}
	TArray<UObject*> Anims = UEditorUtilityLibrary::GetSelectedAssetsOfClass(UAnimSequence::StaticClass());

	for (int32 i = 0; i < Anims.Num(); i++)
	{
		if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(Anims[i]))
		{
			if (UAnimationModifiersAssetUserData* UserData = AnimSequence->GetAssetUserData<UAnimationModifiersAssetUserData>())
			{
				const TArray<UAnimationModifier*>& Modifiers = UserData->GetAnimationModifierInstances().FilterByPredicate([&](const UAnimationModifier* Instance)
				{
					return Instance && Instance->GetClass()->IsChildOf(ModifierClass);
				});

				for (const UAnimationModifier* Modifier : Modifiers)
				{
					if (Modifier)
					{
						Modifier->RevertFromAnimationSequence(AnimSequence);
					}
				}
			}
		}
	}
}

float UGMS_EditorUtilityLibrary::GetSamplingFrameRate(const UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		return AnimSequence->GetSamplingFrameRate().AsDecimal();
	}

	return 0;
}

TArray<FName> UGMS_EditorUtilityLibrary::GetAllCurveNames(const UAnimSequence* AnimSequence)
{
	TArray<FName> Names;
	if (IsValid(AnimSequence))
	{
		for (const FFloatCurve& FloatCurve : AnimSequence->GetCurveData().FloatCurves)
		{
			Names.Add(FloatCurve.GetName());
		}
	}

	return Names;
}


