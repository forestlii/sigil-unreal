// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Settings/GMS_SettingObjectLibrary.h"
#include "Locomotions/GMS_AnimLayer.h"
#include "Animation/BlendSpace.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Locomotions/GMS_AnimLayer_Additive.h"
#include "Locomotions/GMS_AnimLayer_Overlay.h"
#include "Locomotions/GMS_AnimLayer_SkeletalControls.h"
#include "Locomotions/GMS_AnimLayer_States.h"
#include "Locomotions/GMS_AnimLayer_View_Default.h"
#include "Misc/DataValidation.h"
#include "Utility/GMS_Utility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_SettingObjectLibrary)

#pragma region CommonSettings

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"

void UGMS_MovementDefinition::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}

EDataValidationResult UGMS_MovementDefinition::IsDataValid(class FDataValidationContext& Context) const
{
	for (const TTuple<FGameplayTag, FGMS_MovementSetSetting>& Pair : MovementSets)
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


FGameplayTag UGMS_MovementControlSetting_Default::MatchStateTagBySpeed(float Speed, float Threshold) const
{
	for (const FGMS_MovementStateSetting& MovementState : MovementStates)
	{
		if (MovementState.Speed > 0.0f && MovementState.Speed < Speed + Threshold)
		{
			return MovementState.Tag;
		}
	}
	return FGameplayTag::EmptyTag;
}

bool UGMS_MovementControlSetting_Default::GetStateByIndex(const int32& Index, FGMS_MovementStateSetting& OutSetting) const
{
	if (MovementStates.IsValidIndex(Index))
	{
		OutSetting = MovementStates[Index];
		return true;
	}
	return false;
}

bool UGMS_MovementControlSetting_Default::GetStateBySpeedLevel(const int32& Level, FGMS_MovementStateSetting& OutSetting) const
{
	if (SpeedLevelToArrayIndex.Contains(Level))
	{
		OutSetting = MovementStates[SpeedLevelToArrayIndex[Level]];
		return true;
	}
	return false;
}

bool UGMS_MovementControlSetting_Default::GetStateByTag(const FGameplayTag& Tag, FGMS_MovementStateSetting& OutSetting) const
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

void UGMS_MovementControlSetting_Default::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	TagToArrayIndex.Empty();
	SpeedLevelToArrayIndex.Empty();

	MovementStates.Sort([](const FGMS_MovementStateSetting& A, const FGMS_MovementStateSetting& B)
	{
		return A.SpeedLevel < B.SpeedLevel;
	});

	for (int i = 0; i < MovementStates.Num(); ++i)
	{
		FGMS_MovementStateSetting& Setting = MovementStates[i];
		Setting.bVelocityDirection = Setting.AllowedRotationModes.Contains(GMS_RotationModeTags::VelocityDirection);
		Setting.bViewDirection = Setting.AllowedRotationModes.Contains(GMS_RotationModeTags::ViewDirection);

		Setting.EditorFriendlyName = FString::Format(TEXT("State({0}) SpeedLevel({1}) Speed({2})"), {UGMS_Utility::GetSimpleTagName(Setting.Tag).ToString(), Setting.SpeedLevel, Setting.Speed});
		TagToArrayIndex.Emplace(Setting.Tag, i);
		SpeedLevelToArrayIndex.Emplace(Setting.SpeedLevel, i);
	}
}

EDataValidationResult UGMS_MovementControlSetting_Default::IsDataValid(class FDataValidationContext& Context) const
{
	for (int32 i = 0; i < MovementStates.Num(); i++)
	{
		const FGMS_MovementStateSetting& MRSetting = MovementStates[i];
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
