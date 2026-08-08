// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SigilAnimNotify_SendGameplayEvent.generated.h"

/**
 * An anim notify to send gameplay event to owner.
 */
UCLASS()
class SIGILGAS_API USigilAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category="GGA", BlueprintReadWrite)
	FGameplayTag EventTag;
};
