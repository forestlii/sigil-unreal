// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Settings/SigilSettingObjectLibrary.h"
#include "Locomotions/SigilAnimLayer.h"
#include "Animation/BlendSpace.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Locomotions/SigilAnimLayer_Additive.h"
#include "Locomotions/SigilAnimLayer_Overlay.h"
#include "Locomotions/SigilAnimLayer_SkeletalControls.h"
#include "Locomotions/SigilAnimLayer_States.h"
#include "Locomotions/SigilAnimLayer_View_Default.h"
#include "Misc/DataValidation.h"
#include "Utility/SigilUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilSettingObjectLibrary)

#pragma region CommonSettings

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

void USigilMovementDefinition::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}

EDataValidationResult USigilMovementDefinition::IsDataValid(class FDataValidationContext& Context) const
{
	for (const TTuple<FGameplayTag, FSigilMovementSetSetting>& Pair : MovementSets)
	{
		if (Pair.Value.ControlSetting == nullptr)
		{
			Context.AddError(FText::FromString(FString::Format(TEXT("ControlSetting is required on {0}!!!"), {Pair.Key.GetTagName().ToString()})));
		}

		if (!Pair.Value.bUseInstancedStatesSetting && Pair.Value.AnimLayerSetting_States && Pair.Value.AnimLayerSetting_States->IsDataValid(Context) == EDataValidationResult::Invalid)
		{
			return EDataValidationResult::Invalid;
		}

		if (!Pair.Value.bUseInstancedOverlaySetting && Pair.Value.AnimLayerSetting_Overlay && Pair.Value.AnimLayerSetting_Overlay->IsDataValid(Context) == EDataValidationResult::Invalid)
		{
			return EDataValidationResult::Invalid;
		}

		if (Pair.Value.AnimLayerSetting_Additive && Pair.Value.AnimLayerSetting_Additive->IsDataValid(Context) == EDataValidationResult::Invalid)
		{
			return EDataValidationResult::Invalid;
		}

		if (Pair.Value.AnimLayerSetting_View && Pair.Value.AnimLayerSetting_View->IsDataValid(Context) == EDataValidationResult::Invalid)
		{
			return EDataValidationResult::Invalid;
		}

		if (Pair.Value.AnimLayerSetting_SkeletalControls && Pair.Value.AnimLayerSetting_SkeletalControls->IsDataValid(Context) == EDataValidationResult::Invalid)
		{
			return EDataValidationResult::Invalid;
		}

	}
	return Super::IsDataValid(Context);
}

#endif


FGameplayTag USigilMovementControlSetting_Default::MatchStateTagBySpeed(float Speed, float Threshold) const
{
	for (const FSigilMovementStateSetting& MovementState : MovementStates)
	{
		if (MovementState.Speed > 0.0f && MovementState.Speed < Speed + Threshold)
		{
			return MovementState.Tag;
		}
	}
	return FGameplayTag::EmptyTag;
}

bool USigilMovementControlSetting_Default::GetStateByIndex(const int32& Index, FSigilMovementStateSetting& OutSetting) const
{
	if (MovementStates.IsValidIndex(Index))
	{
		OutSetting = MovementStates[Index];
		return true;
	}
	return false;
}

bool USigilMovementControlSetting_Default::GetStateBySpeedLevel(const int32& Level, FSigilMovementStateSetting& OutSetting) const
{
	if (SpeedLevelToArrayIndex.Contains(Level))
	{
		OutSetting = MovementStates[SpeedLevelToArrayIndex[Level]];
		return true;
	}
	return false;
}

bool USigilMovementControlSetting_Default::GetStateByTag(const FGameplayTag& Tag, FSigilMovementStateSetting& OutSetting) const
{
	if (Tag.IsValid() && TagToArrayIndex.Contains(Tag))
	{
		OutSetting = MovementStates[TagToArrayIndex[Tag]];
		return true;
	}
	return false;
}

#pragma endregion

#pragma region ControlSettings

#if WITH_EDITOR

void USigilMovementControlSetting_Default::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	TagToArrayIndex.Empty();
	SpeedLevelToArrayIndex.Empty();

	MovementStates.Sort([](const FSigilMovementStateSetting& A, const FSigilMovementStateSetting& B)
	{
		return A.SpeedLevel < B.SpeedLevel;
	});

	for (int i = 0; i < MovementStates.Num(); ++i)
	{
		FSigilMovementStateSetting& Setting = MovementStates[i];
		Setting.bVelocityDirection = Setting.AllowedRotationModes.Contains(SigilRotationModeTags::VelocityDirection);
		Setting.bViewDirection = Setting.AllowedRotationModes.Contains(SigilRotationModeTags::ViewDirection);

		Setting.EditorFriendlyName = FString::Format(TEXT("State({0}) SpeedLevel({1}) Speed({2})"), {USigilUtility::GetSimpleTagName(Setting.Tag).ToString(), Setting.SpeedLevel, Setting.Speed});
		TagToArrayIndex.Emplace(Setting.Tag, i);
		SpeedLevelToArrayIndex.Emplace(Setting.SpeedLevel, i);
	}
}

EDataValidationResult USigilMovementControlSetting_Default::IsDataValid(class FDataValidationContext& Context) const
{
	for (int32 i = 0; i < MovementStates.Num(); i++)
	{
		const FSigilMovementStateSetting& MRSetting = MovementStates[i];
		if (!MRSetting.Tag.IsValid())
		{
			Context.AddError(FText::FromString(FString::Format(TEXT("Invalid tag at index({0}) of MovementStates"), {i})));
		}
		if (MRSetting.AllowedRotationModes.IsEmpty())
		{
			Context.AddError(FText::FromString(
				FString::Format(TEXT("AllowedRotationModes at index({0}) of MovementStates can't be empty!"), {i})));
		}
	}
	return Super::IsDataValid(Context);
}
#endif

#pragma endregion
