// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utility/SigilUtility.h"

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
#include "Locomotions/SigilAnimLayer.h"
#include "Locomotions/SigilMainAnimInstance.h"
#include "Settings/SigilSettingObjectLibrary.h"
#include "Utility/SigilLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilUtility)

FString USigilUtility::NameToDisplayString(const FName& Name, const bool bNameIsBool)
{
	return FName::NameToDisplayString(Name.ToString(), bNameIsBool);
}

float USigilUtility::GetAnimationCurveValueFromCharacter(const ACharacter* Character, const FName& CurveName)
{
	const auto* Mesh{IsValid(Character) ? Character->GetMesh() : nullptr};
	const auto* AnimationInstance{IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr};

	return IsValid(AnimationInstance) ? AnimationInstance->GetCurveValue(CurveName) : 0.0f;
}

FGameplayTagContainer USigilUtility::GetChildTags(const FGameplayTag& Tag)
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Tag);
}

FName USigilUtility::GetSimpleTagName(const FGameplayTag& Tag)
{
	const auto TagNode{UGameplayTagsManager::Get().FindTagNode(Tag)};

	return TagNode.IsValid() ? TagNode->GetSimpleTagName() : NAME_None;
}

bool USigilUtility::ShouldDisplayDebugForActor(const AActor* Actor, const FName& DisplayName)
{
	const auto* World{IsValid(Actor) ? Actor->GetWorld() : nullptr};
	const auto* PlayerController{IsValid(World) ? World->GetFirstPlayerController() : nullptr};
	auto* Hud{IsValid(PlayerController) ? PlayerController->GetHUD() : nullptr};

	return IsValid(Hud) && Hud->ShouldDisplayDebug(DisplayName) && Hud->GetCurrentDebugTargetActor() == Actor;
}


float USigilUtility::CalculateAnimatedSpeed(const UAnimSequenceBase* AnimSequence)
{
	if (AnimSequence == nullptr)
	{
		UE_LOG(LogSigilMovement, Warning, TEXT("Passed invalid anim sequence"));
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
	UE_LOG(LogSigilMovement, Warning, TEXT("Unable to Calculate animation speed for animation with no root motion delta (%s)."), *GetNameSafe(AnimSequence));
	return 0.0f;
}


UAnimSequence* USigilUtility::SelectAnimationWithFloat(const TArray<FSigilAnimationWithDistance>& Animations, const float& ReferenceValue)
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

bool USigilUtility::ValidatePoseSearchDatabasesChooser(const UChooserTable* ChooserTable, FText& OutMessage)
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

	bool bValidCtx1 = Ctx1 != nullptr && Ctx1->Class != nullptr && Ctx1->Class->IsChildOf(USigilMainAnimInstance::StaticClass());

	if (!bValidCtx1)
	{
		OutMessage = FText::FromString(FString::Format(
			TEXT("ChooserTable({0}): First context must be ContextObjectTypeClass and the class must be Subclass of USigilMainAnimInstance."), {*ChooserTable->GetName()}));
		return false;
	}

	const FContextObjectTypeClass* Ctx2 = ChooserTable->GetContextData()[1].GetPtr<FContextObjectTypeClass>();

	bool bValidCtx2 = Ctx2 != nullptr && Ctx2->Class != nullptr && Ctx2->Class->IsChildOf(USigilAnimLayer::StaticClass());

	if (!bValidCtx2)
	{
		OutMessage = FText::FromString(
			FString::Format(TEXT("ChooserTable({0}): Secondary context must be ContextObjectTypeClass and the class must be Subclass of USigilAnimLayer."), {*ChooserTable->GetName()}));
		return false;
	}
	return true;
}

bool USigilUtility::IsValidPoseSearchDatabasesChooser(const UChooserTable* ChooserTable)
{
	if (!IsValid(ChooserTable))
	{
		return false;
	}
	if (ChooserTable->GetContextData().Num() != 2)
	{
		UE_LOG(LogSigilMovement, Warning, TEXT("ChooserTable(%s):Context is empty, and only allow 2 element!"), *ChooserTable->GetName());
		return false;
	}

	if (ChooserTable->ResultType != EObjectChooserResultType::ObjectResult || ChooserTable->OutputObjectType != UPoseSearchDatabase::StaticClass())
	{
		UE_LOG(LogSigilMovement, Warning, TEXT("ChooserTable(%s):Result type must be ObjectResult and the OutputObjectType must be PoseSearchDatabase."), *ChooserTable->GetName());
		return false;
	}

	const FContextObjectTypeClass* Ctx1 = ChooserTable->GetContextData()[0].GetPtr<FContextObjectTypeClass>();

	bool bValidCtx1 = Ctx1 != nullptr && Ctx1->Class != nullptr && Ctx1->Class->IsChildOf(USigilMainAnimInstance::StaticClass());

	if (!bValidCtx1)
	{
		UE_LOG(LogSigilMovement, Warning, TEXT("ChooserTable(%s): First context must be ContextObjectTypeClass and the class must be Subclass of USigilMainAnimInstance."),
		       *ChooserTable->GetName());
		return false;
	}

	const FContextObjectTypeClass* Ctx2 = ChooserTable->GetContextData()[1].GetPtr<FContextObjectTypeClass>();

	bool bValidCtx2 = Ctx2 != nullptr && Ctx2->Class != nullptr && Ctx2->Class->IsChildOf(USigilAnimLayer::StaticClass());

	if (!bValidCtx2)
	{
		UE_LOG(LogSigilMovement, Warning, TEXT("ChooserTable(%s): Secondary context must be ContextObjectTypeClass and the class must be Subclass of USigilAnimLayer."),
		       *ChooserTable->GetName());
		return false;
	}

	return true;
}

TArray<UPoseSearchDatabase*> USigilUtility::EvaluatePoseSearchDatabasesChooser(const USigilMainAnimInstance* MainAnimInstance, const USigilAnimLayer* AnimLayerInstance,
                                                                              UChooserTable* ChooserTable)
{
	TArray<UPoseSearchDatabase*> Ret;

	if (!IsValid(ChooserTable))
		return Ret;

	// Fallback single context object version
	FChooserEvaluationContext Context;
	Context.AddObjectParam(const_cast<USigilMainAnimInstance*>(MainAnimInstance));
	Context.AddObjectParam(const_cast<USigilAnimLayer*>(AnimLayerInstance));

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

const USigilMovementSetUserSetting* USigilUtility::GetMovementSetUserSetting(const FSigilMovementSetSetting& MovementSetSetting, TSubclassOf<USigilMovementSetUserSetting> DesiredClass)
{
	if (!IsValid(DesiredClass))
	{
		return nullptr;
	}

	for (TObjectPtr<USigilMovementSetUserSetting> UserSetting : MovementSetSetting.UserSettings)
	{
		if (UserSetting->GetClass() == DesiredClass)
		{
			return UserSetting;
		}
	}
	return nullptr;
}

