// Copyright 2024 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GMS_AnimState.h"
#include "Animation/AnimInstance.h"
#include "GMS_AnimGraph_Layering.generated.h"

/**
 * 
 */
UCLASS()
class GENERICMOVEMENTSYSTEM_API UGMS_AnimGraph_Layering : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	virtual void RefreshLayering();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerHeadCurveName{"LayerHead"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerHeadAdditiveCurveName{"LayerHeadAdditive"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerHeadSlotCurveName{"LayerHeadSlot"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmLeft{"LayerArmLeft"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmLeftAdditive{"LayerArmLeftAdditive"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmLeftSlot{"LayerArmLeftSlot"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmLeftLocalSpace{"LayerArmLeftLocalSpace"};


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmRight{"LayerArmRight"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmRightAdditive{"LayerArmRightAdditive"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmRightSlot{"LayerArmRightSlot"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerArmRightLocalSpace{"LayerArmRightLocalSpace"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerHandLeft{"LayerHandLeft"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerHandRight{"LayerHandRight"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerSpine{"LayerSpine"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerSpineAdditive{"LayerSpineAdditive"};
	FName LayerSpineSlot{"LayerSpineSlot"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerPelvis{"LayerPelvis"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerPelvisSlot{"LayerPelvisSlot"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerLegs{"LayerLegs"};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FName LayerLegsSlot{"LayerLegsSlot"};


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	FGMS_AnimState_Layering LayeringState;
};
