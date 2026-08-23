// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SigilAbilityEntitlementTypes.generated.h"

class USigilAbilitySet;

/** Result status for an experimental ability-entitlement reconciliation request. 实验性 Ability entitlement 对账请求的结果状态。 */
UENUM(BlueprintType)
enum class ESigilAbilityReconcileStatus : uint8
{
	Applied,
	Unchanged,
	StaleRevision,
	RevisionConflict,
	NotAuthority,
	ActorInfoNotReady,
	InvalidSnapshot,
	GrantFailed,
	ReentrantCall,
	RuntimeStateMismatch
};

/** One stable entitlement mapped to an already-loaded, ability-only set. 一个稳定 entitlement 到已加载、仅含 Ability 的集合映射。 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilAbilityEntitlementGrant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Ability Entitlement")
	FGameplayTag EntitlementTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Ability Entitlement")
	TObjectPtr<USigilAbilitySet> AbilitySet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Ability Entitlement")
	int32 OverrideLevel = INDEX_NONE;
};

/** Desired runtime projection. Revision is session-local and is never a save schema version. 期望运行时投影；Revision 仅属于当前会话，不是存档 Schema 版本。 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilAbilityEntitlementSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Ability Entitlement")
	int64 Revision = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sigil|Ability Entitlement")
	TArray<FSigilAbilityEntitlementGrant> Grants;
};

/** Observable result of an entitlement reconciliation request. Entitlement 对账请求的可观察结果。 */
USTRUCT(BlueprintType)
struct SIGILGAS_API FSigilAbilityReconcileResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Ability Entitlement")
	ESigilAbilityReconcileStatus Status = ESigilAbilityReconcileStatus::InvalidSnapshot;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Ability Entitlement")
	int64 AcceptedRevision = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Ability Entitlement")
	FString CanonicalDigest;

	UPROPERTY(BlueprintReadOnly, Category = "Sigil|Ability Entitlement")
	FString Error;

	bool IsSuccess() const
	{
		return Status == ESigilAbilityReconcileStatus::Applied || Status == ESigilAbilityReconcileStatus::Unchanged;
	}
};
