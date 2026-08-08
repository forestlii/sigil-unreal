// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DataAsset.h"
#include "SigilUIActionFactory.generated.h"

class USigilUIAction;
/**
 *  提供一种通用的方式为UI对象选择合适的可用操作。
 */
UCLASS(BlueprintType, Const)
class SIGILUI_API USigilUIActionFactory : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="GUIS|UIAction")
	TArray<USigilUIAction*> FindAvailableUIActionsForData(const UObject* Data) const;

protected:
	/**
	 * A list of potential actions for incoming data.
	 * 针对传入数据的潜在可用ui操作。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GUIS", Instanced, meta=(TitleProperty="ActionId"))
	TArray<TObjectPtr<USigilUIAction>> PotentialActions;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
