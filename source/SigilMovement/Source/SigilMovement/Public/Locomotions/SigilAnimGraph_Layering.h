// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SigilAnimState.h"
#include "Animation/AnimInstance.h"
#include "SigilAnimGraph_Layering.generated.h"

/**
 * 
 */
UCLASS()
class SIGILMOVEMENT_API USigilAnimGraph_Layering : public UAnimInstance
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
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
	FSigilAnimState_Layering LayeringState;
};
