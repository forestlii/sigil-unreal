// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once


#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

#include "SigilCameraSystemComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UCanvas;
class AHUD;
class USigilCameraMode;
class USigilCameraModeStack;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;
template <class TClass>
class TSubclassOf;

DECLARE_DELEGATE_RetVal(TSubclassOf<USigilCameraMode>, FGMSCameraModeDelegate);


/**
 * USigilCameraSystemComponent
 *
 *	The base camera component class used by GMS camera system.
 */
UCLASS(ClassGroup=GCMS, meta=(BlueprintSpawnableComponent))
class SIGILCAMERA_API USigilCameraSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USigilCameraSystemComponent(const FObjectInitializer& ObjectInitializer);

	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GCMS|Camera", meta=(DefaultToSelf="Actor"))
	static USigilCameraSystemComponent* GetCameraSystemComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<USigilCameraSystemComponent>() : nullptr); }

	/**
	 * Returns the target actor that the camera is looking at.
	 */
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	// Delegate used to query for the best camera mode.
	FGMSCameraModeDelegate DetermineCameraModeDelegate;

	// Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

	// Push specified Camera Mode.
	UFUNCTION(BlueprintCallable, Category="GCMS|Camera")
	void PushCameraMode(TSubclassOf<USigilCameraMode> NewCameraMode);

	UFUNCTION(BlueprintCallable, Category="GCMS|Camera")
	void PushDefaultCameraMode();

	/**
	 * Initialize it with camera component and sprint arm component. 
	 */
	UFUNCTION(BlueprintCallable, Category="GCMS|Camera")
	void Initialize(UCameraComponent* NewCameraComponent, USpringArmComponent* NewSpringArmComponent);

	virtual void DrawDebug(UCanvas* Canvas) const;

	/**
	 * Gets the camera mode tag associated with the top layer and the blend weight of it
	 * 返回顶层相机模式的tag和当前权重。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCMS|Camera")
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCMS|Camera")
	UCameraComponent* GetAssociatedCamera() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GCMS|Camera")
	USpringArmComponent* GetAssociatedSpringArm() const;

	virtual void Activate(bool bReset) override;
	virtual void Deactivate() override;

protected:
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void UpdateCameraModes();

protected:
	UPROPERTY()
	TObjectPtr<UCameraComponent> AssociatedCameraComponent;

	UPROPERTY()
	TObjectPtr<USpringArmComponent> AssociatedSpringArmComponent;

	// Stack used to blend the camera modes.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="GCMS|Camera", meta=(ShowInnerProperties))
	TObjectPtr<USigilCameraModeStack> CameraModeStack;

	// Default camera mode will used.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GCMS|Camera")
	TSubclassOf<USigilCameraMode> DefaultCameraMode;

	// Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	float FieldOfViewOffset;
};
