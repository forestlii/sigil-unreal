// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GGA_AnimNotify_SendGameplayEvent.generated.h"

/**
 * An anim notify to send gameplay event to owner.
 */
UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_AnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category="GGA", BlueprintReadWrite)
	FGameplayTag EventTag;
};
