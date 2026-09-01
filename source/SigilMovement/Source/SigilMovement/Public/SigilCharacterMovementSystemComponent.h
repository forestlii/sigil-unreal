// Copyright (c) 2026 Likeon. All Rights Reserved.
#pragma once

#include "SigilMovementSystemComponent.h"
#include "GameFramework/Character.h"
#include "SigilCharacterMovementSystemComponent.generated.h"

class USigilCharacterMovementSetting_Default;
class USigilCharacterRotationSetting_Default;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleSignature);

UENUM(BlueprintType)
enum class ESigilMovementRuntimeInitializationMode : uint8
{
	Strict,
	DeferredUntilConfigured
};

UENUM(BlueprintType)
enum class ESigilMovementRotationAuthority : uint8
{
	SigilMovement,
	Controller,
	MovementDirection,
	External
};


/**
 *  SigilMovementComponent
 */
UCLASS(ClassGroup=GMS, BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent), DisplayName="GMS Movement System Component(Character)")
class SIGILMOVEMENT_API USigilCharacterMovementSystemComponent : public USigilMovementSystemComponent
{
	GENERATED_BODY()

public:
	explicit USigilCharacterMovementSystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem|Initialization")
	void SetRuntimeInitializationMode(ESigilMovementRuntimeInitializationMode NewMode);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem|Initialization")
	ESigilMovementRuntimeInitializationMode GetRuntimeInitializationMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem|Initialization")
	bool IsConfiguredRuntimeActive() const;

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem|Initialization")
	bool TryActivateConfiguredRuntime();

	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem|Rotation")
	bool SetRotationAuthority(ESigilMovementRotationAuthority NewAuthority);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GMS|MovementSystem|Rotation")
	ESigilMovementRotationAuthority GetRotationAuthority() const;

	UCharacterMovementComponent* GetCharacterMovement() const { return CharacterMovement; };
	

protected:
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|GameplayTags", meta=( Categories="Sigil.Movement.LocomotionMode"))
	TMap<TEnumAsByte<EMovementMode>, FGameplayTag> MovementModeToTagMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|GameplayTags", meta=(Categories="Sigil.Movement.LocomotionMode"))
	TMap<uint8, FGameplayTag> CustomMovementModeToTagMapping;

	/**
	 * If checked, will apply character movement settings from definition to CharacterMovementComponent.
	 * @attention Sometimes, you want externally control the speed of character(AI), so you can turn it off. 
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|DynamicMovementState")
	bool bAllowRefreshCharacterMovementSettings{true};

	/**
	 * Optional curve mapping ground speed to movement state index(gaits like walk0,jog1,sprint2).
	 * result index will be used to dynamic switch movement state from current available definitions.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|DynamicMovementState", meta=(EditCondition="!bAllowRefreshCharacterMovementSettings"))
	TObjectPtr<UCurveFloat> SpeedToMovementStateCurve;

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UFUNCTION()
	virtual void OnCharacterMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0);

	/**
	 * By default, apply desired movement state setting to character movement component.
	 * This is where GMS primarily change CMC.
	 */
	virtual void ApplyMovementSetting() override;
	virtual void OnMovementSetChanged_Implementation(const FGameplayTag& PreviousMovementSet) override;

	virtual void RefreshInput(float DeltaTime) override;

	virtual void OnRotationModeChanged_Implementation(const FGameplayTag& PreviousRotationMode) override;

#pragma region ViewSystem

protected:
	virtual void RefreshView(float DeltaTime);

#pragma endregion

#pragma region Locomotion

public:
	virtual float GetScaledCapsuleRadius() const override;
	virtual float GetScaledCapsuleHalfHeight() const override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual float GetWalkableFloorZ() const override;
	virtual float GetGravityZ() const override;
	virtual USkeletalMeshComponent* GetMesh() const override;

protected:
	UFUNCTION(BlueprintCallable, Category="GMS|MovementSystem")
	virtual void RefreshLocomotionLocationAndRotation();

	virtual void RefreshLocomotionEarly();

	virtual void RefreshLocomotion(float DeltaTime);

	virtual void RefreshDynamicMovementState();

	virtual void RefreshLocomotionLate(float DeltaTime);
#pragma endregion Locomotion

#pragma region Rotation System

protected:
	virtual void RefreshGroundedRotation(float DeltaTime);

	virtual void RefreshGroundedNotMovingRotation(float DeltaTime);

	virtual void RefreshGroundedMovingRotation(float DeltaTime);

	virtual bool ConstrainAimingRotation(FRotator& ActorRotation, float DeltaTime, bool bApplySecondaryConstraint = false);

	bool ApplyRotationYawSpeedAnimationCurve(float DeltaTime);

	// Hook allow you runs your own rotation logic.If return true, default rotation logic will not run.
	UFUNCTION(BlueprintNativeEvent, Category="GMS|MovementSystem")
	bool RefreshCustomGroundedMovingRotation(float DeltaTime);

	// Hook allow you runs your own rotation logic.If return true, default rotation logic will not run.
	UFUNCTION(BlueprintNativeEvent, Category="GMS|MovementSystem")
	bool RefreshCustomGroundedNotMovingRotation(float DeltaTime);

	virtual float CalculateGroundedRotationInterpolationSpeed() const;

	virtual float CalculateTargetYawRotationSpeed() const;

	virtual void RefreshInAirRotation(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|MovementSystem")
	void SetRotationInstant(const float TargetYawAngle, const ETeleportType Teleport);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|MovementSystem")
	void SetRotationSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GMS|MovementSystem")
	void SetRotationExtraSmooth(float TargetYawAngle, float DeltaTime,
	                            float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed);

	void RefreshTargetYawAngleUsingLocomotionRotation();

	void SetTargetYawAngle(float TargetYawAngle);

	void SetTargetYawAngleSmooth(float TargetYawAngle, float DeltaTime, float RotationSpeed);

	void RefreshViewRelativeTargetYawAngle();
#pragma endregion

#pragma region DistanceMatching
	virtual FSigilPredictGroundMovementPivotLocationParams GetPredictGroundMovementPivotLocationParams() const override;
	virtual FSigilPredictGroundMovementStopLocationParams GetPredictGroundMovementStopLocationParams() const override;
#pragma endregion

private:
	static bool IsSupportedRotationAuthority(
		ESigilMovementRotationAuthority Authority);
	bool ApplyRotationAuthority(ESigilMovementRotationAuthority Authority);
	void StartConfiguredRuntime();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Initialization", meta=(AllowPrivateAccess="true"))
	ESigilMovementRuntimeInitializationMode RuntimeInitializationMode{
		ESigilMovementRuntimeInitializationMode::Strict};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Rotation", meta=(AllowPrivateAccess="true"))
	ESigilMovementRotationAuthority RotationAuthority{
		ESigilMovementRotationAuthority::SigilMovement};

	UPROPERTY(Transient)
	bool bConfiguredRuntimeActive{false};

	UPROPERTY(Transient)
	bool bRotationAuthorityLocked{false};
};
