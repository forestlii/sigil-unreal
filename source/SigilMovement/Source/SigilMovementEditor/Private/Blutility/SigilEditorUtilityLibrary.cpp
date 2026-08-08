// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Blutility/SigilEditorUtilityLibrary.h"

#include "AnimationModifier.h"
#include "AnimationModifiersAssetUserData.h"
#include "EditorUtilityLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilEditorUtilityLibrary)

void USigilEditorUtilityLibrary::RevertAnimModifierOfClass(TSubclassOf<UAnimationModifier> ModifierClass)
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

float USigilEditorUtilityLibrary::GetSamplingFrameRate(const UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		return AnimSequence->GetSamplingFrameRate().AsDecimal();
	}

	return 0;
}

TArray<FName> USigilEditorUtilityLibrary::GetAllCurveNames(const UAnimSequence* AnimSequence)
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


