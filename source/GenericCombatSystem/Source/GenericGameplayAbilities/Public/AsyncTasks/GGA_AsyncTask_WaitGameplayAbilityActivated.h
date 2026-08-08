// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "GGA_AsyncTask_WaitGameplayAbilityActivated.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGGA_AbilityActivatedDelegate, const UGameplayAbility*, Ability);

UCLASS()
class GENERICGAMEPLAYABILITIES_API UGGA_AsyncTask_WaitGameplayAbilityActivated : public UAbilityAsync
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "GGA|Tasks", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UGGA_AsyncTask_WaitGameplayAbilityActivated* WaitGameplayAbilityActivated(AActor* TargetActor);

	void HandleAbilityActivated(UGameplayAbility* Ability);

	UPROPERTY(BlueprintAssignable)
	FGGA_AbilityActivatedDelegate OnAbilityActivated;

protected:
	virtual bool ShouldBroadcastDelegates() const override;
	virtual void Activate() override;
	virtual void EndAction() override;

	FDelegateHandle DelegateHandle;
};
