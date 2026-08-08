// Copyright 2024 RedMoonGames All Rights Reserved.


#include "Utility/GMS_Utility.h"

#include "Chooser.h"
#include "ChooserPropertyAccess.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "GameplayTagsManager.h"
#include "IObjectChooser.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "Animation/AnimSequenceBase.h"
#include "BlendStack/AnimNode_BlendStack.h"
#include "Locomotions/GMS_AnimLayer.h"
#include "Locomotions/GMS_MainAnimInstance.h"
#include "Settings/GMS_SettingObjectLibrary.h"
#include "Utility/GMS_Log.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GMS_Utility)

FString UGMS_Utility::NameToDisplayString(const FName& Name, const bool bNameIsBool)
{
	return FName::NameToDisplayString(Name.ToString(), bNameIsBool);
}

float UGMS_Utility::GetAnimationCurveValueFromCharacter(const ACharacter* Character, const FName& CurveName)
{
	const auto* Mesh{IsValid(Character) ? Character->GetMesh() : nullptr};
	const auto* AnimationInstance{IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr};

	return IsValid(AnimationInstance) ? AnimationInstance->GetCurveValue(CurveName) : 0.0f;
}

FGameplayTagContainer UGMS_Utility::GetChildTags(const FGameplayTag& Tag)
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Tag);
}

FName UGMS_Utility::GetSimpleTagName(const FGameplayTag& Tag)
{
	const auto TagNode{UGameplayTagsManager::Get().FindTagNode(Tag)};

	return TagNode.IsValid() ? TagNode->GetSimpleTagName() : NAME_None;
}

bool UGMS_Utility::ShouldDisplayDebugForActor(const AActor* Actor, const FName& DisplayName)
{
	const auto* World{IsValid(Actor) ? Actor->GetWorld() : nullptr};
	const auto* PlayerController{IsValid(World) ? World->GetFirstPlayerController() : nullptr};
	auto* Hud{IsValid(PlayerController) ? PlayerController->GetHUD() : nullptr};

	return IsValid(Hud) && Hud->ShouldDisplayDebug(DisplayName) && Hud->GetCurrentDebugTargetActor() == Actor;
}


float UGMS_Utility::CalculateAnimatedSpeed(const UAnimSequenceBase* AnimSequence)
{
	if (AnimSequence == nullptr)
	{
		UE_LOG(LogGMS, Warning, TEXT("Passed invalid anim sequence"));
		return 0.0f;
	}

	const float AnimLength = AnimSequence->GetPlayLength();
	// Calculate the speed as: (distance traveled by the animation) / (length of the animation)
	const FVector RootMotionTranslation = AnimSequence->ExtractRootMotionFromRange(0.0, AnimLength, FAnimExtractContext(0.0, true)).GetTranslation();
	const float RootMotionDistance = RootMotionTranslation.Size2D();
	if (!FMath::IsNearlyZero(RootMotionDistance))
	{
		const float AnimationSpeed = RootMotionDistance / AnimLength;
		return AnimationSpeed;
	}
	UE_LOG(LogGMS, Warning, TEXT("Unable to Calculate animation speed for animation with no root motion delta (%s)."), *GetNameSafe(AnimSequence));
	return 0.0f;
}


UAnimSequence* UGMS_Utility::SelectAnimationWithFloat(const TArray<FGMS_AnimationWithDistance>& Animations, const float& ReferenceValue)
{
	if (Animations.IsEmpty())
	{
		return nullptr;
	}

	float Delta = FLT_MAX; // 使用FLT_MAX来初始化Delta
	int32 Found = INDEX_NONE;

	for (int32 i = 0; i < Animations.Num(); i++)
	{
		// 计算与传入Float的绝对差值
		float TempDelta = FMath::Abs(ReferenceValue - Animations[i].Distance); 

		// 如果当前的差值更小，则更新Delta和Found
		if (TempDelta < Delta)
		{
			Delta = TempDelta;
			Found = i;
		}
	}

	// 返回找到的动画或最后一个动画作为备选
	return (Found != INDEX_NONE) ? Animations[Found].Animation : Animations.Last().Animation;
}

bool UGMS_Utility::ValidatePoseSearchDatabasesChooser(const UChooserTable* ChooserTable, FText& OutMessage)
{
	if (!IsValid(ChooserTable))
	{
		OutMessage = FText::FromName("Invalid ChooserTable");
		return false;
	}
	if (ChooserTable->GetContextData().Num() != 2)
	{
		OutMessage = FText::FromString(FString::Format(TEXT("ChooserTable({0}):Context is empty, and only allow 2 element!"), {*ChooserTable->GetName()}));
		return false;
	}

	if (ChooserTable->ResultType != EObjectChooserResultType::ObjectResult || ChooserTable->OutputObjectType != UPoseSearchDatabase::StaticClass())
	{
		OutMessage = FText::FromString(FString::Format(TEXT("ChooserTable({0}):Result type must be ObjectResult and the OutputObjectType must be PoseSearchDatabase."), {*ChooserTable->GetName()}));
		return false;
	}

	const FContextObjectTypeClass* Ctx1 = ChooserTable->GetContextData()[0].GetPtr<FContextObjectTypeClass>();

	bool bValidCtx1 = Ctx1 != nullptr && Ctx1->Class != nullptr && Ctx1->Class->IsChildOf(UGMS_MainAnimInstance::StaticClass());

	if (!bValidCtx1)
	{
		OutMessage = FText::FromString(FString::Format(
			TEXT("ChooserTable({0}): First context must be ContextObjectTypeClass and the class must be Subclass of UGMS_MainAnimInstance."), {*ChooserTable->GetName()}));
		return false;
	}

	const FContextObjectTypeClass* Ctx2 = ChooserTable->GetContextData()[1].GetPtr<FContextObjectTypeClass>();

	bool bValidCtx2 = Ctx2 != nullptr && Ctx2->Class != nullptr && Ctx2->Class->IsChildOf(UGMS_AnimLayer::StaticClass());

	if (!bValidCtx2)
	{
		OutMessage = FText::FromString(
			FString::Format(TEXT("ChooserTable({0}): Secondary context must be ContextObjectTypeClass and the class must be Subclass of UGMS_AnimLayer."), {*ChooserTable->GetName()}));
		return false;
	}
	return true;
}

bool UGMS_Utility::IsValidPoseSearchDatabasesChooser(const UChooserTable* ChooserTable)
{
	if (!IsValid(ChooserTable))
	{
		return false;
	}
	if (ChooserTable->GetContextData().Num() != 2)
	{
		UE_LOG(LogGMS, Warning, TEXT("ChooserTable(%s):Context is empty, and only allow 2 element!"), *ChooserTable->GetName());
		return false;
	}

	if (ChooserTable->ResultType != EObjectChooserResultType::ObjectResult || ChooserTable->OutputObjectType != UPoseSearchDatabase::StaticClass())
	{
		UE_LOG(LogGMS, Warning, TEXT("ChooserTable(%s):Result type must be ObjectResult and the OutputObjectType must be PoseSearchDatabase."), *ChooserTable->GetName());
		return false;
	}

	const FContextObjectTypeClass* Ctx1 = ChooserTable->GetContextData()[0].GetPtr<FContextObjectTypeClass>();

	bool bValidCtx1 = Ctx1 != nullptr && Ctx1->Class != nullptr && Ctx1->Class->IsChildOf(UGMS_MainAnimInstance::StaticClass());

	if (!bValidCtx1)
	{
		UE_LOG(LogGMS, Warning, TEXT("ChooserTable(%s): First context must be ContextObjectTypeClass and the class must be Subclass of UGMS_MainAnimInstance."),
		       *ChooserTable->GetName());
		return false;
	}

	const FContextObjectTypeClass* Ctx2 = ChooserTable->GetContextData()[1].GetPtr<FContextObjectTypeClass>();

	bool bValidCtx2 = Ctx2 != nullptr && Ctx2->Class != nullptr && Ctx2->Class->IsChildOf(UGMS_AnimLayer::StaticClass());

	if (!bValidCtx2)
	{
		UE_LOG(LogGMS, Warning, TEXT("ChooserTable(%s): Secondary context must be ContextObjectTypeClass and the class must be Subclass of UGMS_AnimLayer."),
		       *ChooserTable->GetName());
		return false;
	}

	return true;
}

TArray<UPoseSearchDatabase*> UGMS_Utility::EvaluatePoseSearchDatabasesChooser(const UGMS_MainAnimInstance* MainAnimInstance, const UGMS_AnimLayer* AnimLayerInstance,
                                                                              UChooserTable* ChooserTable)
{
	TArray<UPoseSearchDatabase*> Ret;

	if (!IsValid(ChooserTable))
		return Ret;

	// Fallback single context object version
	FChooserEvaluationContext Context;
	Context.AddObjectParam(const_cast<UGMS_MainAnimInstance*>(MainAnimInstance));
	Context.AddObjectParam(const_cast<UGMS_AnimLayer*>(AnimLayerInstance));

	auto Callback = FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda([&Ret](UObject* InResult)
	{
		if (InResult && InResult->IsA(UPoseSearchDatabase::StaticClass()))
		{
			Ret.Add(Cast<UPoseSearchDatabase>(InResult));
		}
		return FObjectChooserBase::EIteratorStatus::Continue;
	});

	UChooserTable::EvaluateChooser(Context, ChooserTable, Callback);
	return Ret;
}

const UGMS_MovementSetUserSetting* UGMS_Utility::GetMovementSetUserSetting(const FGMS_MovementSetSetting& MovementSetSetting, TSubclassOf<UGMS_MovementSetUserSetting> DesiredClass)
{
	if (!IsValid(DesiredClass))
	{
		return nullptr;
	}

	for (TObjectPtr<UGMS_MovementSetUserSetting> UserSetting : MovementSetSetting.UserSettings)
	{
		if (UserSetting->GetClass() == DesiredClass)
		{
			return UserSetting;
		}
	}
	return nullptr;
}

// void UGMS_Utility::BlendToWithSettings(const FAnimUpdateContext& Context, const FBlendStackAnimNodeReference& BlendStackNode, UAnimationAsset* AnimationAsset, float AnimationTime, bool bLoop,
// 	bool bMirrored, float BlendTime, UBlendProfile* BlendProfile, EAlphaBlendOption BlendOption, bool bInertialBlend, FVector BlendParameters, float WantedPlayRate, float ActivationDelay,
// 	FName SyncGroupName, EAnimGroupRole::Type SyncGroupRole, EAnimSyncMethod SyncGroupMethod)
// {
// 	if (AnimationAsset != nullptr)
// 	{
// 		if (FAnimNode_BlendStack* BlendStackNodePtr = BlendStackNode.GetAnimNodePtr<FAnimNode_BlendStack>())
// 		{
// 			if (const FAnimationUpdateContext* AnimationUpdateContext = Context.GetContext())
// 			{
// 				BlendStackNodePtr->BlendTo(
// 					*AnimationUpdateContext,
// 					AnimationAsset,
// 					AnimationTime,
// 					bLoop,
// 					bMirrored,
// 					BlendStackNodePtr->MirrorDataTable,
// 					BlendTime,
// 					BlendProfile,
// 					BlendOption,
// 					bInertialBlend,
// 					BlendParameters,
// 					WantedPlayRate,
// 					ActivationDelay,
// 					SyncGroupName, SyncGroupRole, SyncGroupMethod);
// 			}
// 			else
// 			{
// 				UE_LOG(LogGMS, Warning, TEXT("UBlendStackAnimNodeLibrary::BlendToWithSettings called with an invalid context."));
// 			}
// 		}
// 		else
// 		{
// 			UE_LOG(LogGMS, Warning, TEXT("UBlendStackAnimNodeLibrary::BlendToWithSettings called with an invalid type."));
// 		}
// 	}
// }

