// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Locomotions/SigilLocomotionStructLibrary.h"
#include "Settings/SigilSettingStructLibrary.h"
#include "SigilUtility.generated.h"

class USigilMovementSetUserSetting;
class USigilAnimLayer;
class USigilMainAnimInstance;
class UChooserTable;
class UPoseSearchDatabase;
class UAnimSequenceBase;

DECLARE_STATS_GROUP(TEXT("GMS"), STATGROUP_GMS, STATCAT_Advanced)

UCLASS()
class SIGILMOVEMENT_API USigilUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr auto DrawImpactPointSize{32.0f};
	static constexpr auto DrawLineThickness{1.0f};
	static constexpr auto DrawArrowSize{50.0f};
	static constexpr auto DrawCircleSidesCount{16};

public:
	static constexpr FStringView BoolToString(bool bValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Utility", Meta = (AutoCreateRefTerm = "Name", ReturnDisplayName = "Display String"))
	static FString NameToDisplayString(const FName& Name, bool bNameIsBool);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Utility",
		Meta = (DefaultToSelf = "Character", AutoCreateRefTerm = "CurveName", ReturnDisplayName = "Curve Value"))
	static float GetAnimationCurveValueFromCharacter(const ACharacter* Character, const FName& CurveName);

	UFUNCTION(BlueprintCallable, Category="GMS|Utility", Meta = (AutoCreateRefTerm = "Tag", ReturnDisplayName = "Child Tags"))
	static FGameplayTagContainer GetChildTags(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Utility", Meta = (AutoCreateRefTerm = "Tag", ReturnDisplayName = "Tag Name"))
	static FName GetSimpleTagName(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Utility",
		Meta = (DefaultToSelf = "Actor", AutoCreateRefTerm = "DisplayName", ReturnDisplayName = "Value"))
	static bool ShouldDisplayDebugForActor(const AActor* Actor, const FName& DisplayName);

	/**
	 * Root-motion 2D speed of the whole sequence (distance / length). Returns 0 for null sequences or sequences without
	 * root motion delta. Thread-safe and log-free — it extracts root motion over the full range, so cache the result
	 * per asset instead of calling it every frame.
	 * 整段序列的根运动 2D 速度（位移 / 时长）。序列为空或无根运动位移时返回 0。线程安全且不打日志——它会提取
	 * 整段根运动，请按资产缓存结果，不要每帧调用。
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	static float CalculateAnimatedSpeed(const UAnimSequenceBase* AnimSequence);

	/**
	 * 
	 * @param Animations List of Animations to select.
	 * @param ReferenceValue The animation with distance closest to ReferenceValue will be selected.
	 * @return Selected AnimSequence.
	 */
	UFUNCTION(BlueprintCallable, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	static UAnimSequence* SelectAnimationWithFloat(const TArray<FSigilAnimationWithDistance>& Animations, const float& ReferenceValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	static bool ValidatePoseSearchDatabasesChooser(const UChooserTable* ChooserTable, FText& OutMessage);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	static bool IsValidPoseSearchDatabasesChooser(const UChooserTable* ChooserTable);

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="GMS|Animation", meta=(BlueprintThreadSafe))
	static TArray<UPoseSearchDatabase*> EvaluatePoseSearchDatabasesChooser(const USigilMainAnimInstance* MainAnimInstance, const USigilAnimLayer* AnimLayerInstance, UChooserTable* ChooserTable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|Utility", meta=(BlueprintThreadSafe,DeterminesOutputType=DesiredClass,DynamicOutputParam="ReturnValue"))
	static const USigilMovementSetUserSetting* GetMovementSetUserSetting(const FSigilMovementSetSetting& MovementSetSetting, TSubclassOf<USigilMovementSetUserSetting> DesiredClass);
};

constexpr FStringView USigilUtility::BoolToString(const bool bValue)
{
	return bValue ? TEXTVIEW("True") : TEXTVIEW("False");
}
