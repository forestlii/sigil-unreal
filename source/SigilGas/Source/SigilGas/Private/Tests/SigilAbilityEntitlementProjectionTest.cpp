// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbility_CharacterJump.h"
#include "Abilities/GameplayAbility_Montage.h"
#include "Abilities/SigilAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"
#include "SigilAbilitySystemComponent.h"
#include "SigilGasTags.h"
#include "Tests/AutomationCommon.h"
#include "UObject/UObjectGlobals.h"

/**
 * 单文件 Spike 测试接缝。生产头只声明此 friend；所有故障注入和私有状态读取都留在测试编译中。
 */
struct FSigilAbilityEntitlementProjectionTestAccess
{
	static void AddAbility(
		USigilAbilitySet& AbilitySet,
		TSubclassOf<UGameplayAbility> AbilityClass,
		int32 AbilityLevel = 1,
		int32 InputID = INDEX_NONE,
		FGameplayTag DynamicTag = FGameplayTag())
	{
		FSigilAbilitySet_GameplayAbility& Entry = AbilitySet.GrantedGameplayAbilities.AddDefaulted_GetRef();
		Entry.Ability = AbilityClass.Get();
		Entry.AbilityLevel = AbilityLevel;
		Entry.InputID = InputID;
		if (DynamicTag.IsValid())
		{
			Entry.DynamicTags.AddTag(DynamicTag);
		}
#if WITH_EDITORONLY_DATA
		Entry.bAbilityEnabled = true;
#endif
	}

	static void AddUnresolvedAbility(USigilAbilitySet& AbilitySet, const FSoftObjectPath& AbilityClassPath)
	{
		FSigilAbilitySet_GameplayAbility& Entry = AbilitySet.GrantedGameplayAbilities.AddDefaulted_GetRef();
		Entry.Ability = AbilityClassPath;
		Entry.AbilityLevel = 1;
#if WITH_EDITORONLY_DATA
		Entry.bAbilityEnabled = true;
#endif
	}

	static void AddGameplayEffect(USigilAbilitySet& AbilitySet, TSubclassOf<UGameplayEffect> EffectClass)
	{
		FSigilAbilitySet_GameplayEffect& Entry = AbilitySet.GrantedGameplayEffects.AddDefaulted_GetRef();
		Entry.GameplayEffect = EffectClass.Get();
#if WITH_EDITORONLY_DATA
		Entry.bEffectEnabled = true;
#endif
	}

	static void AddAttributeSet(USigilAbilitySet& AbilitySet, TSubclassOf<UAttributeSet> AttributeSetClass)
	{
		FSigilAbilitySet_AttributeSet& Entry = AbilitySet.GrantedAttributes.AddDefaulted_GetRef();
		Entry.AttributeSet = AttributeSetClass.Get();
#if WITH_EDITORONLY_DATA
		Entry.bAttributeSetEnabled = true;
#endif
	}

	static bool IsAbilityClassLoaded(const USigilAbilitySet& AbilitySet, int32 AbilityIndex)
	{
		return AbilitySet.GrantedGameplayAbilities.IsValidIndex(AbilityIndex)
			&& AbilitySet.GrantedGameplayAbilities[AbilityIndex].Ability.IsValid();
	}

	static bool IsAbilityClassNull(const USigilAbilitySet& AbilitySet, int32 AbilityIndex)
	{
		return !AbilitySet.GrantedGameplayAbilities.IsValidIndex(AbilityIndex)
			|| AbilitySet.GrantedGameplayAbilities[AbilityIndex].Ability.IsNull();
	}

	static int32 GetRuntimeGrantCount(const USigilAbilitySystemComponent& ASC)
	{
		return ASC.AbilityEntitlementRuntimeGrants.Num();
	}

	static int32 GetEntitlementCount(const USigilAbilitySystemComponent& ASC)
	{
		return ASC.AbilityEntitlementToIdentity.Num();
	}

	static int32 GetContributorCount(const USigilAbilitySystemComponent& ASC, FGameplayTag EntitlementTag)
	{
		const FString* Identity = ASC.AbilityEntitlementToIdentity.Find(EntitlementTag);
		if (!Identity)
		{
			return 0;
		}
		const USigilAbilitySystemComponent::FAbilityEntitlementRuntimeGrant* RuntimeGrant = ASC.AbilityEntitlementRuntimeGrants.Find(*Identity);
		return RuntimeGrant ? RuntimeGrant->Contributors.Num() : 0;
	}

	static int32 GetProjectionHandleCount(const USigilAbilitySystemComponent& ASC)
	{
		int32 Count = 0;
		for (const TPair<FString, USigilAbilitySystemComponent::FAbilityEntitlementRuntimeGrant>& RuntimePair : ASC.AbilityEntitlementRuntimeGrants)
		{
			Count += RuntimePair.Value.GrantedHandles.AbilitySpecHandles.Num();
		}
		return Count;
	}

	static FGameplayAbilitySpecHandle GetFirstProjectionHandle(const USigilAbilitySystemComponent& ASC)
	{
		for (const TPair<FString, USigilAbilitySystemComponent::FAbilityEntitlementRuntimeGrant>& RuntimePair : ASC.AbilityEntitlementRuntimeGrants)
		{
			if (!RuntimePair.Value.GrantedHandles.AbilitySpecHandles.IsEmpty())
			{
				return RuntimePair.Value.GrantedHandles.AbilitySpecHandles[0];
			}
		}
		return FGameplayAbilitySpecHandle();
	}

	static int32 GetGateSourceCount(const USigilAbilitySystemComponent& ASC, FGameplayTag GateTag)
	{
		const TSet<FName>* Sources = ASC.AbilityActivationGateSources.Find(GateTag);
		return Sources ? Sources->Num() : 0;
	}

	static bool HasGateSource(const USigilAbilitySystemComponent& ASC, FGameplayTag GateTag, FName SourceId)
	{
		const TSet<FName>* Sources = ASC.AbilityActivationGateSources.Find(GateTag);
		return Sources && Sources->Contains(SourceId);
	}

	static int32 GetGateCount(const USigilAbilitySystemComponent& ASC)
	{
		return ASC.AbilityActivationGateSources.Num();
	}

	static bool NeedsProjectionReset(const USigilAbilitySystemComponent& ASC)
	{
		return ASC.bAbilityEntitlementProjectionNeedsReset;
	}

	static int32 GetExplicitTagCount(const USigilAbilitySystemComponent& ASC, FGameplayTag Tag)
	{
		return ASC.GameplayTagCountContainer.GetExplicitTagCount(Tag);
	}

	static int64 GetProjectionEpoch(const USigilAbilitySystemComponent& ASC)
	{
		return ASC.AbilityEntitlementProjectionEpoch;
	}

	static int64 GetAcceptedRevision(const USigilAbilitySystemComponent& ASC)
	{
		return ASC.bHasAcceptedAbilityEntitlementSnapshot ? ASC.LastAcceptedAbilityEntitlementRevision : INDEX_NONE;
	}

	static FString GetAcceptedDigest(const USigilAbilitySystemComponent& ASC)
	{
		return ASC.bHasAcceptedAbilityEntitlementSnapshot ? ASC.LastAcceptedAbilityEntitlementDigest : FString();
	}

	static void SetGrantFailureOrdinal(USigilAbilitySystemComponent& ASC, int32 GrantOrdinal)
	{
		ASC.AbilityEntitlementFailGrantOrdinalForTest = GrantOrdinal;
	}

	static int32 GetGrantedHandleCount(const FSigilAbilitySet_GrantedHandles& Handles)
	{
		return Handles.AbilitySpecHandles.Num();
	}

	static FGameplayAbilitySpecHandle GetFirstGrantedHandle(const FSigilAbilitySet_GrantedHandles& Handles)
	{
		return Handles.AbilitySpecHandles.IsEmpty() ? FGameplayAbilitySpecHandle() : Handles.AbilitySpecHandles[0];
	}
};

namespace
{
	struct FTestAscContext
	{
		AActor* Owner = nullptr;
		USigilAbilitySystemComponent* ASC = nullptr;
	};

	struct FDurableGateFixture
	{
		FGameplayTag GateTag;
		FName SourceId;
	};

	/**
	 * 仅用于此 Automation 的 durable Desired 夹具：保存稳定 Tag、已加载 AbilitySet 映射和 gate 来源，
	 * 不引用 Project 类型，也不模拟不存在的异步存档或迟到 callback。
	 */
	struct FDurableDesiredFixture
	{
		int64 Revision = 0;
		TArray<FSigilAbilityEntitlementGrant> Grants;
		TArray<FDurableGateFixture> Gates;

		FSigilAbilityReconcileResult ApplyTo(USigilAbilitySystemComponent& ASC) const
		{
			FSigilAbilityEntitlementSnapshot Snapshot;
			Snapshot.Revision = Revision;
			Snapshot.Grants = Grants;
			FSigilAbilityReconcileResult Result = ASC.ReconcileAbilityEntitlements(Snapshot);
			if (Result.IsSuccess())
			{
				for (const FDurableGateFixture& Gate : Gates)
				{
					ASC.SetAbilityActivationGateSource(Gate.GateTag, Gate.SourceId, true);
				}
			}
			return Result;
		}
	};

	FTestAscContext CreateAsc(UWorld& World, ENetRole Role = ROLE_Authority)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags |= RF_Transient;
		AActor* Owner = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParameters);
		if (!Owner)
		{
			return {};
		}
		Owner->SetRole(Role);

		USigilAbilitySystemComponent* ASC = NewObject<USigilAbilitySystemComponent>(Owner, NAME_None, RF_Transient);
		Owner->AddInstanceComponent(ASC);
		ASC->RegisterComponent();
		ASC->InitAbilityActorInfo(Owner, Owner);
		return {Owner, ASC};
	}

	USigilAbilitySet* CreateAbilitySet(const TCHAR* BaseName)
	{
		const FName UniqueName = MakeUniqueObjectName(GetTransientPackage(), USigilAbilitySet::StaticClass(), FName(BaseName));
		return NewObject<USigilAbilitySet>(GetTransientPackage(), UniqueName, RF_Transient);
	}

	FSigilAbilityEntitlementGrant MakeGrant(
		FGameplayTag EntitlementTag,
		USigilAbilitySet* AbilitySet,
		int32 OverrideLevel = INDEX_NONE)
	{
		FSigilAbilityEntitlementGrant Grant;
		Grant.EntitlementTag = EntitlementTag;
		Grant.AbilitySet = AbilitySet;
		Grant.OverrideLevel = OverrideLevel;
		return Grant;
	}

	FSigilAbilityEntitlementSnapshot MakeSnapshot(
		int64 Revision,
		std::initializer_list<FSigilAbilityEntitlementGrant> Grants)
	{
		FSigilAbilityEntitlementSnapshot Snapshot;
		Snapshot.Revision = Revision;
		Snapshot.Grants.Append(Grants.begin(), static_cast<int32>(Grants.size()));
		return Snapshot;
	}

	bool TestStatus(
		FAutomationTestBase& Test,
		const FString& What,
		const FSigilAbilityReconcileResult& Result,
		ESigilAbilityReconcileStatus Expected)
	{
		return Test.TestEqual(
			*What,
			static_cast<uint8>(Result.Status),
			static_cast<uint8>(Expected));
	}

	int32 CountSpecsOfClass(const USigilAbilitySystemComponent& ASC, UClass* AbilityClass)
	{
		int32 Count = 0;
		for (const FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
			{
				++Count;
			}
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilAbilityEntitlementProjectionTest,
	"SigilGas.AbilityEntitlement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilAbilityEntitlementProjectionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FTestWorldWrapper WorldWrapper;
	// EditorPreview 不创建 GameInstance，避免与本测试无关的 Host UI subsystem 配置错误污染结果。
	if (!WorldWrapper.CreateTestWorld(EWorldType::EditorPreview))
	{
		WorldWrapper.ForwardErrorMessages(this);
		return false;
	}
	UWorld* World = WorldWrapper.GetTestWorld();
	if (!TestNotNull(TEXT("创建临时测试 World"), World))
	{
		return false;
	}

	const FGameplayTag EntitlementA = SigilStateTags::Interacting;
	const FGameplayTag EntitlementB = SigilStateTags::InteractingRemoval;
	const FGameplayTag EntitlementC = SigilAbilityTraitTags::Persistent;
	const FGameplayTag ExternalTag = SigilAbilityTraitTags::ActivationOnSpawn;
	const FGameplayTag GateTag = SigilAbilityActivateFailTags::Networking;
	const FGameplayTag InputExactTag = SigilAbilityActivateFailTags::TagsBlocked;
	const FGameplayTag InputOtherTag = SigilAbilityActivateFailTags::Cost;
	const FGameplayTag InputParentTag = UGameplayTagsManager::Get().RequestGameplayTag(
		FName(TEXT("Sigil.Ability.ActivateFail")), false);

	TestTrue(TEXT("测试 entitlement tags 有效"),
		EntitlementA.IsValid() && EntitlementB.IsValid() && EntitlementC.IsValid());
	TestTrue(TEXT("测试 gate/external/input tags 有效"),
		GateTag.IsValid() && ExternalTag.IsValid()
		&& InputExactTag.IsValid() && InputOtherTag.IsValid() && InputParentTag.IsValid());

	// Revision、canonical digest 排序与连续十次幂等。
	{
		const FTestAscContext Context = CreateAsc(*World);
		if (!TestNotNull(TEXT("Revision 场景 ASC"), Context.ASC))
		{
			return false;
		}

		USigilAbilitySet* SetA = CreateAbilitySet(TEXT("EntitlementRevisionSetA"));
		USigilAbilitySet* SetB = CreateAbilitySet(TEXT("EntitlementRevisionSetB"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*SetA, UGameplayAbility::StaticClass(), 2);
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*SetB, UGameplayAbility_CharacterJump::StaticClass(), 4);

		const FSigilAbilityEntitlementSnapshot Ordered = MakeSnapshot(5,
			{MakeGrant(EntitlementA, SetA), MakeGrant(EntitlementB, SetB)});
		const FSigilAbilityEntitlementSnapshot Reordered = MakeSnapshot(5,
			{MakeGrant(EntitlementB, SetB), MakeGrant(EntitlementA, SetA)});
		const FSigilAbilityReconcileResult Applied = Context.ASC->ReconcileAbilityEntitlements(Ordered);
		TestStatus(*this, TEXT("首个有序快照 Applied"), Applied, ESigilAbilityReconcileStatus::Applied);
		TestEqual(TEXT("首个快照两份 spec"), Context.ASC->GetActivatableAbilities().Num(), 2);
		TestFalse(TEXT("首个 canonical digest 非空"), Applied.CanonicalDigest.IsEmpty());

		const FGameplayAbilitySpec* StableSpecA = Context.ASC->FindAbilitySpecFromClass(UGameplayAbility::StaticClass());
		const FGameplayAbilitySpec* StableSpecB = Context.ASC->FindAbilitySpecFromClass(UGameplayAbility_CharacterJump::StaticClass());
		const FGameplayAbilitySpecHandle StableHandleA = StableSpecA ? StableSpecA->Handle : FGameplayAbilitySpecHandle();
		const FGameplayAbilitySpecHandle StableHandleB = StableSpecB ? StableSpecB->Handle : FGameplayAbilitySpecHandle();
		TestTrue(TEXT("幂等前两份 handle 有效"), StableHandleA.IsValid() && StableHandleB.IsValid());

		for (int32 Iteration = 0; Iteration < 10; ++Iteration)
		{
			const FSigilAbilityReconcileResult Repeat = Context.ASC->ReconcileAbilityEntitlements(Reordered);
			TestStatus(*this, FString::Printf(TEXT("排序后第 %d 次 reconcile Unchanged"), Iteration + 1),
				Repeat, ESigilAbilityReconcileStatus::Unchanged);
			TestEqual(*FString::Printf(TEXT("排序后第 %d 次 digest 稳定"), Iteration + 1),
				Repeat.CanonicalDigest, Applied.CanonicalDigest);
			TestEqual(*FString::Printf(TEXT("排序后第 %d 次 spec 数稳定"), Iteration + 1),
				Context.ASC->GetActivatableAbilities().Num(), 2);
		}
		TestEqual(TEXT("十次幂等后 A handle 未替换"),
			Context.ASC->FindAbilitySpecFromClass(UGameplayAbility::StaticClass())->Handle, StableHandleA);
		TestEqual(TEXT("十次幂等后 B handle 未替换"),
			Context.ASC->FindAbilitySpecFromClass(UGameplayAbility_CharacterJump::StaticClass())->Handle, StableHandleB);

		const FSigilAbilityReconcileResult Stale = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(4, {MakeGrant(EntitlementA, SetA), MakeGrant(EntitlementB, SetB)}));
		TestStatus(*this, TEXT("低 Revision 被拒绝"), Stale, ESigilAbilityReconcileStatus::StaleRevision);

		const FSigilAbilityReconcileResult Conflict = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(5, {MakeGrant(EntitlementA, SetA), MakeGrant(EntitlementB, SetB, 7)}));
		TestStatus(*this, TEXT("同 Revision 异 payload 冲突"), Conflict, ESigilAbilityReconcileStatus::RevisionConflict);
		TestEqual(TEXT("stale/conflict 后 revision 不变"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(5));
		TestEqual(TEXT("stale/conflict 后 digest 不变"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC), Applied.CanonicalDigest);

		const FSigilAbilityReconcileResult Higher = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(6, {MakeGrant(EntitlementA, SetA)}));
		TestStatus(*this, TEXT("更高 Revision 改变投影"), Higher, ESigilAbilityReconcileStatus::Applied);
		TestEqual(TEXT("更高 Revision 移除旧 identity"), Context.ASC->GetActivatableAbilities().Num(), 1);
		TestEqual(TEXT("保留 identity 的 handle 不变"),
			Context.ASC->FindAbilitySpecFromClass(UGameplayAbility::StaticClass())->Handle, StableHandleA);

		Context.ASC->SetAbilityActivationGateSource(GateTag, FName(TEXT("StableBeforeInvalid")), true);
		const int32 SpecsBeforeInvalid = Context.ASC->GetActivatableAbilities().Num();
		const int32 RuntimeBeforeInvalid = FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC);
		const int32 EntitlementsBeforeInvalid = FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC);
		const FString DigestBeforeInvalid = FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC);
		const FSigilAbilityReconcileResult Invalid = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(7, {MakeGrant(EntitlementC, nullptr)}));
		TestStatus(*this, TEXT("preflight invalid 被拒绝"), Invalid, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("preflight invalid 不改 spec"), Context.ASC->GetActivatableAbilities().Num(), SpecsBeforeInvalid);
		TestEqual(TEXT("preflight invalid 不改 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), RuntimeBeforeInvalid);
		TestEqual(TEXT("preflight invalid 不改 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), EntitlementsBeforeInvalid);
		TestEqual(TEXT("preflight invalid 不改 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(6));
		TestEqual(TEXT("preflight invalid 不改 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC), DigestBeforeInvalid);
		TestEqual(TEXT("preflight invalid 不改 gate source"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateTag), 1);
		TestEqual(TEXT("preflight invalid 不改 gate tag count"), Context.ASC->GetTagCount(GateTag), 1);
	}

	// Entitlement 投影只接受纯 Ability Set；夹带 Effect 或 Attribute 必须在 preflight 零变化拒绝。
	{
		const FTestAscContext EffectContext = CreateAsc(*World);
		USigilAbilitySet* EffectSet = CreateAbilitySet(TEXT("EntitlementSetWithEffect"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*EffectSet, UGameplayAbility::StaticClass(), 1);
		FSigilAbilityEntitlementProjectionTestAccess::AddGameplayEffect(
			*EffectSet, UGameplayEffect::StaticClass());
		const FSigilAbilityReconcileResult EffectResult = EffectContext.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, EffectSet)}));
		TestStatus(*this, TEXT("夹带 Effect 的 Set 被拒绝"),
			EffectResult, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("夹带 Effect 不产生 spec"), EffectContext.ASC->GetActivatableAbilities().Num(), 0);
		TestEqual(TEXT("夹带 Effect 不产生 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*EffectContext.ASC), 0);
		TestEqual(TEXT("夹带 Effect 不产生 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*EffectContext.ASC), 0);
		TestEqual(TEXT("夹带 Effect 不产生 projection handle"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*EffectContext.ASC), 0);
		TestEqual(TEXT("夹带 Effect 不接受 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*EffectContext.ASC), int64(INDEX_NONE));
		TestTrue(TEXT("夹带 Effect 不接受 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*EffectContext.ASC).IsEmpty());

		const FTestAscContext AttributeContext = CreateAsc(*World);
		USigilAbilitySet* AttributeSet = CreateAbilitySet(TEXT("EntitlementSetWithAttribute"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*AttributeSet, UGameplayAbility::StaticClass(), 1);
		FSigilAbilityEntitlementProjectionTestAccess::AddAttributeSet(
			*AttributeSet, UAttributeSet::StaticClass());
		const FSigilAbilityReconcileResult AttributeResult = AttributeContext.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, AttributeSet)}));
		TestStatus(*this, TEXT("夹带 Attribute 的 Set 被拒绝"),
			AttributeResult, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("夹带 Attribute 不产生 spec"), AttributeContext.ASC->GetActivatableAbilities().Num(), 0);
		TestEqual(TEXT("夹带 Attribute 不产生 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*AttributeContext.ASC), 0);
		TestEqual(TEXT("夹带 Attribute 不产生 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*AttributeContext.ASC), 0);
		TestEqual(TEXT("夹带 Attribute 不产生 projection handle"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*AttributeContext.ASC), 0);
		TestEqual(TEXT("夹带 Attribute 不接受 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*AttributeContext.ASC), int64(INDEX_NONE));
		TestTrue(TEXT("夹带 Attribute 不接受 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*AttributeContext.ASC).IsEmpty());
	}

	// 同一快照不允许重复 EntitlementTag，即使每份 grant 单独看都有效。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* DuplicateTagSetA = CreateAbilitySet(TEXT("EntitlementDuplicateTagSetA"));
		USigilAbilitySet* DuplicateTagSetB = CreateAbilitySet(TEXT("EntitlementDuplicateTagSetB"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*DuplicateTagSetA, UGameplayAbility::StaticClass(), 1);
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*DuplicateTagSetB, UGameplayAbility_CharacterJump::StaticClass(), 1);
		const FSigilAbilityReconcileResult Result = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1,
				{MakeGrant(EntitlementA, DuplicateTagSetA), MakeGrant(EntitlementA, DuplicateTagSetB)}));
		TestStatus(*this, TEXT("重复 EntitlementTag 被拒绝"),
			Result, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("重复 EntitlementTag 不产生 spec"), Context.ASC->GetActivatableAbilities().Num(), 0);
		TestEqual(TEXT("重复 EntitlementTag 不产生 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 0);
		TestEqual(TEXT("重复 EntitlementTag 不产生 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 0);
		TestEqual(TEXT("重复 EntitlementTag 不产生 projection handle"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*Context.ASC), 0);
		TestEqual(TEXT("重复 EntitlementTag 不接受 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(INDEX_NONE));
		TestTrue(TEXT("重复 EntitlementTag 不接受 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC).IsEmpty());
	}

	// 单个 Set 内重复 AbilityClass 必须拒绝，避免一个 identity 内出现歧义句柄。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* DuplicateClassSet = CreateAbilitySet(TEXT("EntitlementDuplicateClassSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*DuplicateClassSet, UGameplayAbility::StaticClass(), 1);
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*DuplicateClassSet, UGameplayAbility::StaticClass(), 2);
		const FSigilAbilityReconcileResult Result = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, DuplicateClassSet)}));
		TestStatus(*this, TEXT("同一 Set 重复 AbilityClass 被拒绝"),
			Result, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("重复 AbilityClass 不产生 spec"), Context.ASC->GetActivatableAbilities().Num(), 0);
		TestEqual(TEXT("重复 AbilityClass 不产生 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 0);
		TestEqual(TEXT("重复 AbilityClass 不产生 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 0);
		TestEqual(TEXT("重复 AbilityClass 不产生 projection handle"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*Context.ASC), 0);
		TestEqual(TEXT("重复 AbilityClass 不接受 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(INDEX_NONE));
		TestTrue(TEXT("重复 AbilityClass 不接受 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC).IsEmpty());
	}

	// 外部 grant 已拥有同 AbilityClass 时 fail closed，且不能触碰外部 handle。
	{
		const FTestAscContext Context = CreateAsc(*World);
		FGameplayAbilitySpec ExternalSpec(
			UGameplayAbility::StaticClass()->GetDefaultObject<UGameplayAbility>(), 5);
		ExternalSpec.SourceObject = Context.Owner;
		const FGameplayAbilitySpecHandle ExternalHandle = Context.ASC->GiveAbility(ExternalSpec);
		TestTrue(TEXT("外部同 class spec handle 有效"), ExternalHandle.IsValid());

		USigilAbilitySet* ConflictingSet = CreateAbilitySet(TEXT("EntitlementExternalClassConflictSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*ConflictingSet, UGameplayAbility::StaticClass(), 5);
		const FSigilAbilityReconcileResult Result = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, ConflictingSet)}));
		TestStatus(*this, TEXT("外部同 AbilityClass spec 冲突被拒绝"),
			Result, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("外部同 class 冲突后仍只有外部 spec"),
			Context.ASC->GetActivatableAbilities().Num(), 1);
		const FGameplayAbilitySpec* ExternalSpecAfter = Context.ASC->FindAbilitySpecFromHandle(ExternalHandle);
		TestNotNull(TEXT("外部同 class 冲突后原 handle 仍有效"), ExternalSpecAfter);
		if (ExternalSpecAfter)
		{
			TestEqual(TEXT("外部同 class 冲突后 level 不变"), ExternalSpecAfter->Level, 5);
			TestTrue(TEXT("外部同 class 冲突后 SourceObject 不变"),
				ExternalSpecAfter->SourceObject.Get() == Context.Owner);
		}
		TestEqual(TEXT("外部同 class 冲突不产生 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 0);
		TestEqual(TEXT("外部同 class 冲突不产生 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 0);
		TestEqual(TEXT("外部同 class 冲突不产生 projection handle"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*Context.ASC), 0);
		TestEqual(TEXT("外部同 class 冲突不接受 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(INDEX_NONE));
		TestTrue(TEXT("外部同 class 冲突不接受 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC).IsEmpty());
	}

	// 新 entitlement 路径只接受已加载 class；未解析 soft path 必须保持未加载并零变化。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* UnresolvedSet = CreateAbilitySet(TEXT("EntitlementUnresolvedSet"));
		const FSoftObjectPath NeverLoadedPath(TEXT("/Game/SigilGasTests/NeverLoadedAbility.NeverLoadedAbility_C"));
		FSigilAbilityEntitlementProjectionTestAccess::AddUnresolvedAbility(*UnresolvedSet, NeverLoadedPath);
		TestFalse(TEXT("未解析 soft class 不是空引用"),
			FSigilAbilityEntitlementProjectionTestAccess::IsAbilityClassNull(*UnresolvedSet, 0));
		TestFalse(TEXT("reconcile 前 soft class 未加载"),
			FSigilAbilityEntitlementProjectionTestAccess::IsAbilityClassLoaded(*UnresolvedSet, 0));
		const FSigilAbilityReconcileResult Result = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, UnresolvedSet)}));
		TestStatus(*this, TEXT("未加载 class 在 preflight fail closed"), Result, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestFalse(TEXT("reconcile 后 soft class 仍未加载"),
			FSigilAbilityEntitlementProjectionTestAccess::IsAbilityClassLoaded(*UnresolvedSet, 0));
		TestEqual(TEXT("未加载 class 路径不产生 spec"), Context.ASC->GetActivatableAbilities().Num(), 0);
	}

	// 多个 entitlement 对同一 canonical identity 共享 grant，最后 contributor 才撤销。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* SharedSet = CreateAbilitySet(TEXT("EntitlementSharedSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*SharedSet, UGameplayAbility::StaticClass(), 3);
		TestStatus(*this, TEXT("共享 identity 初始投影"), Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, SharedSet), MakeGrant(EntitlementB, SharedSet)})),
			ESigilAbilityReconcileStatus::Applied);
		TestEqual(TEXT("共享 identity 只有一个 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 1);
		TestEqual(TEXT("共享 identity 只有一份 spec"), Context.ASC->GetActivatableAbilities().Num(), 1);
		TestEqual(TEXT("共享 identity contributor=2"),
			FSigilAbilityEntitlementProjectionTestAccess::GetContributorCount(*Context.ASC, EntitlementA), 2);
		const FGameplayAbilitySpecHandle SharedHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC);

		TestStatus(*this, TEXT("移除一个 contributor"), Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(2, {MakeGrant(EntitlementB, SharedSet)})), ESigilAbilityReconcileStatus::Applied);
		TestEqual(TEXT("剩余 contributor 保留 spec"), Context.ASC->GetActivatableAbilities().Num(), 1);
		TestEqual(TEXT("剩余 contributor=1"),
			FSigilAbilityEntitlementProjectionTestAccess::GetContributorCount(*Context.ASC, EntitlementB), 1);
		TestEqual(TEXT("共享 grant handle 未重建"),
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC), SharedHandle);

		TestStatus(*this, TEXT("移除最后 contributor"),
			Context.ASC->ReconcileAbilityEntitlements(MakeSnapshot(3, {})), ESigilAbilityReconcileStatus::Applied);
		TestEqual(TEXT("最后 contributor 后 spec 撤销"), Context.ASC->GetActivatableAbilities().Num(), 0);
		TestEqual(TEXT("最后 contributor 后 runtime grant 清空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 0);
	}

	// 同一 AbilityClass 通过不同 identity 到达必须在 preflight 阶段拒绝。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* IdentityOne = CreateAbilitySet(TEXT("EntitlementIdentityOne"));
		USigilAbilitySet* IdentityTwo = CreateAbilitySet(TEXT("EntitlementIdentityTwo"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*IdentityOne, UGameplayAbility::StaticClass(), 1);
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*IdentityTwo, UGameplayAbility::StaticClass(), 2);
		const FSigilAbilityReconcileResult Result = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, IdentityOne), MakeGrant(EntitlementB, IdentityTwo)}));
		TestStatus(*this, TEXT("不兼容 identity 被拒绝"), Result, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("不兼容 identity 不产生 spec"), Context.ASC->GetActivatableAbilities().Num(), 0);
		TestEqual(TEXT("不兼容 identity 不产生 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 0);
	}

	// 跨 Revision 也不能把同一 AbilityClass 从既有 identity 静默换成不兼容的新 identity。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* StableIdentitySet = CreateAbilitySet(TEXT("EntitlementStableIdentitySet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*StableIdentitySet, UGameplayAbility::StaticClass(), 1);

		const FSigilAbilityReconcileResult RevisionOne = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, StableIdentitySet)}));
		TestStatus(*this, TEXT("跨 Revision identity 基线 Applied"),
			RevisionOne, ESigilAbilityReconcileStatus::Applied);
		const FGameplayAbilitySpecHandle StableHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC);
		const FString StableDigest =
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC);
		Context.ASC->SetAbilityActivationGateSource(
			GateTag, FName(TEXT("StableAcrossIdentityConflict")), true);

		const FSigilAbilityReconcileResult RevisionTwoConflict = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(2, {MakeGrant(EntitlementA, StableIdentitySet, 7)}));
		TestStatus(*this, TEXT("跨 Revision 同 class 异 identity 被拒绝"),
			RevisionTwoConflict, ESigilAbilityReconcileStatus::InvalidSnapshot);
		TestEqual(TEXT("跨 Revision identity 冲突结果保留 accepted revision"),
			RevisionTwoConflict.AcceptedRevision, int64(1));
		TestEqual(TEXT("跨 Revision identity 冲突结果保留 accepted digest"),
			RevisionTwoConflict.CanonicalDigest, StableDigest);
		TestEqual(TEXT("跨 Revision identity 冲突不推进内部 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(1));
		TestEqual(TEXT("跨 Revision identity 冲突不改变内部 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC), StableDigest);
		TestEqual(TEXT("跨 Revision identity 冲突仍只有旧 spec"),
			Context.ASC->GetActivatableAbilities().Num(), 1);
		TestNotNull(TEXT("跨 Revision identity 冲突后旧 handle 仍有效"),
			Context.ASC->FindAbilitySpecFromHandle(StableHandle));
		TestEqual(TEXT("跨 Revision identity 冲突不替换旧 handle"),
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC), StableHandle);
		const FGameplayAbilitySpec* StableSpec = Context.ASC->FindAbilitySpecFromHandle(StableHandle);
		if (StableSpec)
		{
			TestEqual(TEXT("跨 Revision identity 冲突不改变旧 spec level"), StableSpec->Level, 1);
		}
		TestEqual(TEXT("跨 Revision identity 冲突不改变 runtime grant"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 1);
		TestEqual(TEXT("跨 Revision identity 冲突不改变 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 1);
		TestEqual(TEXT("跨 Revision identity 冲突不改变 contributor"),
			FSigilAbilityEntitlementProjectionTestAccess::GetContributorCount(*Context.ASC, EntitlementA), 1);
		TestEqual(TEXT("跨 Revision identity 冲突不改变 projection handle 数"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*Context.ASC), 1);
		TestEqual(TEXT("跨 Revision identity 冲突不改变 gate source"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateTag), 1);
		TestEqual(TEXT("跨 Revision identity 冲突不改变 gate tag count"),
			Context.ASC->GetTagCount(GateTag), 1);
	}

	// INDEX_NONE 与显式写出 entry 默认 level 必须归一为同一 canonical identity。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* CanonicalLevelSet = CreateAbilitySet(TEXT("EntitlementCanonicalLevelSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*CanonicalLevelSet, UGameplayAbility::StaticClass(), 3);

		const FSigilAbilityReconcileResult ImplicitLevel = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, CanonicalLevelSet, INDEX_NONE)}));
		TestStatus(*this, TEXT("隐式 entry level 基线 Applied"),
			ImplicitLevel, ESigilAbilityReconcileStatus::Applied);
		const FGameplayAbilitySpecHandle StableCanonicalHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC);
		TestTrue(TEXT("隐式 entry level handle 有效"), StableCanonicalHandle.IsValid());

		const FSigilAbilityReconcileResult ExplicitSameLevel = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(2, {MakeGrant(EntitlementA, CanonicalLevelSet, 3)}));
		TestStatus(*this, TEXT("显式相同 effective level 的更高 Revision Applied"),
			ExplicitSameLevel, ESigilAbilityReconcileStatus::Applied);
		TestEqual(TEXT("显式默认 level 与隐式 level digest 相同"),
			ExplicitSameLevel.CanonicalDigest, ImplicitLevel.CanonicalDigest);
		TestEqual(TEXT("显式相同 effective level 接受 Revision 2"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(2));
		TestEqual(TEXT("显式相同 effective level 仍只有一份 spec"),
			Context.ASC->GetActivatableAbilities().Num(), 1);
		TestNotNull(TEXT("显式相同 effective level 后旧 handle 仍有效"),
			Context.ASC->FindAbilitySpecFromHandle(StableCanonicalHandle));
		TestEqual(TEXT("显式相同 effective level 不 churn handle"),
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC), StableCanonicalHandle);
		const FGameplayAbilitySpec* StableCanonicalSpec =
			Context.ASC->FindAbilitySpecFromHandle(StableCanonicalHandle);
		if (StableCanonicalSpec)
		{
			TestEqual(TEXT("归一后 spec level 保持 3"), StableCanonicalSpec->Level, 3);
		}
		TestEqual(TEXT("显式相同 effective level 仍只有一个 runtime identity"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 1);
		TestEqual(TEXT("显式相同 effective level entitlement index 单一"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 1);
		TestEqual(TEXT("显式相同 effective level contributor 单一"),
			FSigilAbilityEntitlementProjectionTestAccess::GetContributorCount(*Context.ASC, EntitlementA), 1);
		TestEqual(TEXT("显式相同 effective level projection handle 单一"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*Context.ASC), 1);
	}

	// 第 N 条 grant 失败时补偿本批句柄，旧投影与已接受 revision/digest 保持不变。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* ExistingSet = CreateAbilitySet(TEXT("EntitlementExistingSet"));
		USigilAbilitySet* NewSet = CreateAbilitySet(TEXT("EntitlementNewSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*ExistingSet, UGameplayAbility::StaticClass(), 1);
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*NewSet, UGameplayAbility_CharacterJump::StaticClass(), 2);
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*NewSet, UGameplayAbility_Montage::StaticClass(), 3);

		const FSigilAbilityReconcileResult ExistingResult = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, ExistingSet)}));
		TestStatus(*this, TEXT("故障注入前旧投影 Applied"), ExistingResult, ESigilAbilityReconcileStatus::Applied);
		const FGameplayAbilitySpecHandle ExistingHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC);
		const FString ExistingDigest = FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC);

		FSigilAbilityEntitlementProjectionTestAccess::SetGrantFailureOrdinal(*Context.ASC, 2);
		const FSigilAbilityReconcileResult Failed = Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(2, {MakeGrant(EntitlementA, ExistingSet), MakeGrant(EntitlementB, NewSet)}));
		TestStatus(*this, TEXT("第二条 grant 故障返回 GrantFailed"), Failed, ESigilAbilityReconcileStatus::GrantFailed);
		TestEqual(TEXT("补偿后只保留旧 spec"), Context.ASC->GetActivatableAbilities().Num(), 1);
		TestNotNull(TEXT("补偿后旧 handle 仍有效"), Context.ASC->FindAbilitySpecFromHandle(ExistingHandle));
		TestEqual(TEXT("补偿后 runtime grant 仍为旧投影"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 1);
		TestEqual(TEXT("补偿后 entitlement index 仍为旧投影"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 1);
		TestEqual(TEXT("补偿后 revision 不推进"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(1));
		TestEqual(TEXT("补偿后 digest 不推进"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC), ExistingDigest);

		FSigilAbilityEntitlementProjectionTestAccess::SetGrantFailureOrdinal(*Context.ASC, INDEX_NONE);
		TestStatus(*this, TEXT("清除故障后同一目标可重试"), Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(2, {MakeGrant(EntitlementA, ExistingSet), MakeGrant(EntitlementB, NewSet)})),
			ESigilAbilityReconcileStatus::Applied);
		TestEqual(TEXT("重试成功后三份 spec"), Context.ASC->GetActivatableAbilities().Num(), 3);
	}

	// Reset 只能清除 entitlement 投影的句柄和 gate 贡献，外部 spec/loose tag 必须保留。
	{
		const FTestAscContext Context = CreateAsc(*World);
		FGameplayAbilitySpec ExternalSpec(UGameplayAbility_Montage::StaticClass()->GetDefaultObject<UGameplayAbility>(), 1);
		const FGameplayAbilitySpecHandle ExternalHandle = Context.ASC->GiveAbility(ExternalSpec);
		Context.ASC->AddLooseGameplayTag(ExternalTag);

		USigilAbilitySet* ProjectionSet = CreateAbilitySet(TEXT("EntitlementResetSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*ProjectionSet, UGameplayAbility::StaticClass(), 1);
		TestStatus(*this, TEXT("Reset 场景 entitlement 投影"), Context.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, ProjectionSet)})), ESigilAbilityReconcileStatus::Applied);
		Context.ASC->SetAbilityActivationGateSource(GateTag, FName(TEXT("Equipment")), true);
		const FGameplayAbilitySpecHandle ProjectionHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC);
		const int64 EpochBeforeReset = FSigilAbilityEntitlementProjectionTestAccess::GetProjectionEpoch(*Context.ASC);
		TestEqual(TEXT("Reset 前外部+投影两份 spec"), Context.ASC->GetActivatableAbilities().Num(), 2);
		TestEqual(TEXT("Reset 前投影 GateTag 只由 SourceId API 贡献"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateTag), 1);
		TestEqual(TEXT("Reset 前独立 ExternalTag 只有外部贡献"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, ExternalTag), 1);

		Context.ASC->ResetAbilityEntitlementProjection();
		TestNotNull(TEXT("Reset 后外部 spec 保留"), Context.ASC->FindAbilitySpecFromHandle(ExternalHandle));
		TestNull(TEXT("Reset 后投影 spec 清除"), Context.ASC->FindAbilitySpecFromHandle(ProjectionHandle));
		TestEqual(TEXT("Reset 后只剩外部 spec"), Context.ASC->GetActivatableAbilities().Num(), 1);
		TestEqual(TEXT("Reset 后投影 GateTag 清除"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateTag), 0);
		TestEqual(TEXT("Reset 后独立 ExternalTag 外部贡献保留"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, ExternalTag), 1);
		TestEqual(TEXT("Reset 后投影 runtime 清空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 0);
		TestEqual(TEXT("Reset 后投影 gate index 清空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 0);
		TestEqual(TEXT("Reset 推进 projection epoch"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionEpoch(*Context.ASC), EpochBeforeReset + 1);
		Context.ASC->ClearAbility(ExternalHandle);
		Context.ASC->RemoveLooseGameplayTag(ExternalTag);
	}

	// test-only durable Desired fixture 在新 ASC B 从零重建；只证明同步重建和组件隔离，不伪造异步。
	{
		const FTestAscContext ContextA = CreateAsc(*World);
		const FTestAscContext ContextB = CreateAsc(*World);
		USigilAbilitySet* DurableSet = CreateAbilitySet(TEXT("EntitlementDurableSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*DurableSet, UGameplayAbility::StaticClass(), 5);

		FDurableDesiredFixture Fixture;
		Fixture.Revision = 42;
		Fixture.Grants = {MakeGrant(EntitlementA, DurableSet), MakeGrant(EntitlementB, DurableSet)};
		Fixture.Gates = {
			{GateTag, FName(TEXT("DurableEquipment"))},
			{GateTag, FName(TEXT("DurableTutorial"))}};

		TestStatus(*this, TEXT("ASC A 从 durable fixture 投影"), Fixture.ApplyTo(*ContextA.ASC),
			ESigilAbilityReconcileStatus::Applied);
		const FGameplayAbilitySpecHandle OldAHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*ContextA.ASC);
		TestEqual(TEXT("ASC A 共享 contributor=2"),
			FSigilAbilityEntitlementProjectionTestAccess::GetContributorCount(*ContextA.ASC, EntitlementA), 2);
		TestEqual(TEXT("ASC A gate sources=2"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*ContextA.ASC, GateTag), 2);

		ContextA.ASC->ResetAbilityEntitlementProjection();
		TestNull(TEXT("ASC A teardown 后旧 handle 无效"), ContextA.ASC->FindAbilitySpecFromHandle(OldAHandle));
		TestEqual(TEXT("ASC A teardown 后 gate 来源清空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*ContextA.ASC, GateTag), 0);

		TestStatus(*this, TEXT("ASC B 从同一 durable fixture 从零重建"), Fixture.ApplyTo(*ContextB.ASC),
			ESigilAbilityReconcileStatus::Applied);
		const FGameplayAbilitySpecHandle NewBHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*ContextB.ASC);
		TestTrue(TEXT("ASC B 新 handle 有效"), NewBHandle.IsValid());
		TestTrue(TEXT("ASC B 不复用 ASC A handle"), NewBHandle != OldAHandle);
		TestEqual(TEXT("ASC B 只有一份共享 spec"), ContextB.ASC->GetActivatableAbilities().Num(), 1);
		TestEqual(TEXT("ASC B contributor=2"),
			FSigilAbilityEntitlementProjectionTestAccess::GetContributorCount(*ContextB.ASC, EntitlementB), 2);
		TestEqual(TEXT("ASC B gate sources=2"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*ContextB.ASC, GateTag), 2);
		TestEqual(TEXT("ASC B gate tag count=1"), ContextB.ASC->GetTagCount(GateTag), 1);

		// 这里只验证旧 A handle/组件局部操作不能触碰 B；候选 API 没有真实异步操作，因此不伪造迟到 callback。
		ContextA.ASC->ClearAbility(OldAHandle);
		ContextA.ASC->SetAbilityActivationGateSource(GateTag, FName(TEXT("DurableEquipment")), false);
		TestNotNull(TEXT("对 ASC A 的旧 handle 操作不触碰 B"), ContextB.ASC->FindAbilitySpecFromHandle(NewBHandle));
		TestEqual(TEXT("对 ASC A 的 gate 操作不触碰 B"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*ContextB.ASC, GateTag), 2);
		AddInfo(TEXT("边界：本用例未伪造异步存档或迟到 callback；真实异步生命周期仍需后续具有真实异步 API 的测试证明。"));
	}

	// Gate 的 (GateTag, SourceId) 幂等、引用计数、输入校验与 Authority fail-closed。
	{
		const FTestAscContext Server = CreateAsc(*World);
		Server.ASC->SetAbilityActivationGateSource(FGameplayTag(), FName(TEXT("InvalidTag")), true);
		Server.ASC->SetAbilityActivationGateSource(GateTag, NAME_None, true);
		TestEqual(TEXT("无效 GateTag/SourceId 零变化"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Server.ASC), 0);

		const FName SourceA(TEXT("EquipmentA"));
		const FName SourceB(TEXT("EquipmentB"));
		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceA, true);
		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceA, true);
		TestEqual(TEXT("重复 enable 同 SourceId 幂等"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Server.ASC, GateTag), 1);
		TestEqual(TEXT("第一个 source 只增加一次 tag"), Server.ASC->GetTagCount(GateTag), 1);

		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceB, true);
		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceB, true);
		TestEqual(TEXT("不同 SourceId 分别计数"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Server.ASC, GateTag), 2);
		TestEqual(TEXT("多个 source 仍只有一个最终 gate tag"), Server.ASC->GetTagCount(GateTag), 1);

		Server.ASC->SetAbilityActivationGateSource(GateTag, FName(TEXT("UnknownSource")), false);
		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceA, false);
		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceA, false);
		TestEqual(TEXT("未知/重复 disable 不下溢"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Server.ASC, GateTag), 1);
		TestEqual(TEXT("仍有 source 时 gate 保留"), Server.ASC->GetTagCount(GateTag), 1);

		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceB, false);
		Server.ASC->SetAbilityActivationGateSource(GateTag, SourceB, false);
		TestEqual(TEXT("最后 source disable 后来源清空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Server.ASC, GateTag), 0);
		TestEqual(TEXT("最后 source disable 后 gate 移除"), Server.ASC->GetTagCount(GateTag), 0);

		const FTestAscContext Client = CreateAsc(*World, ROLE_SimulatedProxy);
		Client.ASC->SetAbilityActivationGateSource(GateTag, SourceA, true);
		TestEqual(TEXT("非 Authority gate 调用零来源"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Client.ASC, GateTag), 0);
		TestEqual(TEXT("非 Authority gate 调用零 tag"), Client.ASC->GetTagCount(GateTag), 0);

		USigilAbilitySet* ClientSet = CreateAbilitySet(TEXT("EntitlementClientAuthoritySet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(*ClientSet, UGameplayAbility::StaticClass(), 1);
		TestStatus(*this, TEXT("非 Authority reconcile fail closed"), Client.ASC->ReconcileAbilityEntitlements(
			MakeSnapshot(1, {MakeGrant(EntitlementA, ClientSet)})), ESigilAbilityReconcileStatus::NotAuthority);
		TestEqual(TEXT("非 Authority reconcile 零 spec"), Client.ASC->GetActivatableAbilities().Num(), 0);
		AddInfo(TEXT("边界：此单进程用例只验证 server/authority 语义；真实复制连接上的客户端可见 gate 未在本用例证明。"));
	}

	// Gate tag 的同步 count delegate 不得重入 Gate/Reset/Reconcile，也不能留下来源或 loose tag orphan。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* ProjectionSet = CreateAbilitySet(TEXT("EntitlementGateDelegateReentrySet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*ProjectionSet, UGameplayAbility::StaticClass(), 2);
		const FSigilAbilityEntitlementSnapshot StableSnapshot =
			MakeSnapshot(1, {MakeGrant(EntitlementA, ProjectionSet)});
		const FSigilAbilityReconcileResult StableApplied =
			Context.ASC->ReconcileAbilityEntitlements(StableSnapshot);
		TestStatus(*this, TEXT("Gate delegate 重入前投影 Applied"),
			StableApplied, ESigilAbilityReconcileStatus::Applied);
		const FGameplayAbilitySpecHandle StableHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC);
		const int64 StableEpoch =
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionEpoch(*Context.ASC);
		const FString StableDigest =
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC);

		const FName SourceA(TEXT("DelegateOuterSourceA"));
		const FName SourceB(TEXT("DelegateReentrantSourceB"));
		int32 EnableCallbackCount = 0;
		int32 DisableCallbackCount = 0;
		int32 UnexpectedCallbackCount = 0;
		bool bEnableReentryAttempted = false;
		bool bDisableReentryAttempted = false;
		bool bOnlyExpectedTag = true;
		FSigilAbilityReconcileResult ReentrantReconcileResult;
		const FSigilAbilityEntitlementSnapshot ReentrantSnapshot =
			MakeSnapshot(2, {MakeGrant(EntitlementA, ProjectionSet)});

		const FDelegateHandle GateDelegateHandle = Context.ASC->RegisterGameplayTagEvent(
			GateTag, EGameplayTagEventType::NewOrRemoved).AddLambda(
			[&](const FGameplayTag ChangedTag, int32 NewCount)
			{
				bOnlyExpectedTag = bOnlyExpectedTag && ChangedTag == GateTag;
				if (NewCount > 0)
				{
					++EnableCallbackCount;
					if (!bEnableReentryAttempted)
					{
						bEnableReentryAttempted = true;
						Context.ASC->SetAbilityActivationGateSource(GateTag, SourceB, true);
						Context.ASC->ResetAbilityEntitlementProjection();
						ReentrantReconcileResult =
							Context.ASC->ReconcileAbilityEntitlements(ReentrantSnapshot);
					}
					return;
				}

				if (NewCount == 0)
				{
					++DisableCallbackCount;
					if (!bDisableReentryAttempted)
					{
						bDisableReentryAttempted = true;
						Context.ASC->SetAbilityActivationGateSource(GateTag, SourceB, true);
					}
					return;
				}

				++UnexpectedCallbackCount;
			});

		Context.ASC->SetAbilityActivationGateSource(GateTag, SourceA, true);
		TestTrue(TEXT("首次 enable delegate 已尝试三种重入"), bEnableReentryAttempted);
		TestEqual(TEXT("首次 enable delegate 只触发一次"), EnableCallbackCount, 1);
		TestEqual(TEXT("首次 enable 未触发 disable delegate"), DisableCallbackCount, 0);
		TestEqual(TEXT("Gate delegate 没有异常 count"), UnexpectedCallbackCount, 0);
		TestTrue(TEXT("Gate delegate 只收到目标 Tag"), bOnlyExpectedTag);
		TestStatus(*this, TEXT("Gate delegate 内 reconcile 被 guard 拒绝"),
			ReentrantReconcileResult, ESigilAbilityReconcileStatus::ReentrantCall);
		TestEqual(TEXT("Gate delegate 内 reconcile 保留 accepted revision"),
			ReentrantReconcileResult.AcceptedRevision, int64(1));
		TestEqual(TEXT("Gate delegate 内 reconcile 保留 accepted digest"),
			ReentrantReconcileResult.CanonicalDigest, StableDigest);
		TestTrue(TEXT("首次 enable 外层只记录 SourceA"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateTag, SourceA));
		TestFalse(TEXT("首次 enable delegate 无法插入 SourceB"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateTag, SourceB));
		TestEqual(TEXT("首次 enable 后来源账本只有一项"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateTag), 1);
		TestEqual(TEXT("首次 enable 后只有一个 gate 账本键"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 1);
		TestEqual(TEXT("首次 enable 后 loose tag count 与账本一致"), Context.ASC->GetTagCount(GateTag), 1);
		TestNotNull(TEXT("Gate delegate 内 Reset 未移除旧 handle"),
			Context.ASC->FindAbilitySpecFromHandle(StableHandle));
		TestEqual(TEXT("Gate delegate 内 Reset 未改变 projection epoch"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionEpoch(*Context.ASC), StableEpoch);
		TestEqual(TEXT("Gate delegate 重入未推进内部 revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(1));
		TestEqual(TEXT("Gate delegate 重入未改变内部 digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC), StableDigest);

		Context.ASC->SetAbilityActivationGateSource(GateTag, SourceA, false);
		TestTrue(TEXT("最后 disable delegate 已尝试重入 enable SourceB"), bDisableReentryAttempted);
		TestEqual(TEXT("最后 disable delegate 只触发一次"), DisableCallbackCount, 1);
		TestEqual(TEXT("最后 disable 未递归触发 enable delegate"), EnableCallbackCount, 1);
		TestFalse(TEXT("最后 disable 后 SourceA 已清除"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateTag, SourceA));
		TestFalse(TEXT("最后 disable delegate 无法重建 SourceB"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateTag, SourceB));
		TestEqual(TEXT("最后 disable 后来源账本无 orphan"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateTag), 0);
		TestEqual(TEXT("最后 disable 后 gate 账本键清空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 0);
		TestEqual(TEXT("最后 disable 后 loose tag 无 orphan"), Context.ASC->GetTagCount(GateTag), 0);
		TestEqual(TEXT("Gate delegate 重入后 runtime grant 仍单一"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 1);
		TestEqual(TEXT("Gate delegate 重入后 entitlement index 仍单一"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 1);
		TestEqual(TEXT("Gate delegate 重入后 projection handle 仍单一"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*Context.ASC), 1);
		TestEqual(TEXT("Gate delegate 重入后旧 handle 未 churn"),
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC), StableHandle);
		TestEqual(TEXT("Gate delegate 重入后仍只有旧投影 spec"),
			Context.ASC->GetActivatableAbilities().Num(), 1);

		TestTrue(TEXT("Gate tag delegate 可确定性注销"),
			Context.ASC->UnregisterGameplayTagEvent(
				GateDelegateHandle, GateTag, EGameplayTagEventType::NewOrRemoved));
		TestStatus(*this, TEXT("Gate 外层完成后同 Revision 仍可幂等 reconcile"),
			Context.ASC->ReconcileAbilityEntitlements(StableSnapshot),
			ESigilAbilityReconcileStatus::Unchanged);
	}

	// Gate source ledger 需要独占其 explicit loose-tag 贡献；外部预占时必须 fail closed 并由 Reset 恢复。
	{
		const FTestAscContext Context = CreateAsc(*World);
		Context.ASC->AddLooseGameplayTag(GateTag, 1);
		TestEqual(TEXT("外部预占时 gate source map 为空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 0);
		TestEqual(TEXT("外部预占提供一份 explicit tag count"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateTag), 1);
		TestEqual(TEXT("外部预占提供一份总 tag count"), Context.ASC->GetTagCount(GateTag), 1);
		TestFalse(TEXT("外部预占尚未要求 projection reset"),
			FSigilAbilityEntitlementProjectionTestAccess::NeedsProjectionReset(*Context.ASC));

		USigilAbilitySet* ProjectionSet = CreateAbilitySet(TEXT("EntitlementGateExclusiveTagSet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*ProjectionSet, UGameplayAbility::StaticClass(), 3);
		TestStatus(*this, TEXT("Gate exclusive-tag 前置投影 Applied"),
			Context.ASC->ReconcileAbilityEntitlements(
				MakeSnapshot(1, {MakeGrant(EntitlementA, ProjectionSet)})),
			ESigilAbilityReconcileStatus::Applied);
		const FGameplayAbilitySpecHandle ProjectionHandle =
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstProjectionHandle(*Context.ASC);
		const int64 EpochBeforeReset =
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionEpoch(*Context.ASC);
		TestEqual(TEXT("前置投影不认领外部 gate tag"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 0);
		TestEqual(TEXT("前置投影不改变外部 explicit count"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateTag), 1);

		const FName SourceA(TEXT("ExclusiveTagSourceA"));
		AddExpectedErrorPlain(
			TEXT("SetAbilityActivationGateSource requires exclusive ownership of the gate tag's explicit loose contribution; the source ledger and explicit count disagree, so an authoritative reset is required."),
			EAutomationExpectedErrorFlags::Exact,
			1);
		Context.ASC->SetAbilityActivationGateSource(GateTag, SourceA, true);
		TestTrue(TEXT("外部 explicit tag 与空 ledger 冲突会置 NeedsReset"),
			FSigilAbilityEntitlementProjectionTestAccess::NeedsProjectionReset(*Context.ASC));
		TestEqual(TEXT("exclusive-tag preflight 不创建 gate map"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 0);
		TestEqual(TEXT("exclusive-tag preflight 不创建 gate source"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateTag), 0);
		TestFalse(TEXT("exclusive-tag preflight 不记录 SourceA"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateTag, SourceA));
		TestEqual(TEXT("exclusive-tag preflight 不改变外部 explicit count"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateTag), 1);
		TestEqual(TEXT("exclusive-tag preflight 不改变外部总 count"), Context.ASC->GetTagCount(GateTag), 1);
		TestNotNull(TEXT("exclusive-tag preflight 尚未触碰投影 handle"),
			Context.ASC->FindAbilitySpecFromHandle(ProjectionHandle));

		Context.ASC->ResetAbilityEntitlementProjection();
		TestEqual(TEXT("authoritative Reset 不扣外部 explicit count"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateTag), 1);
		TestEqual(TEXT("authoritative Reset 不扣外部总 tag count"), Context.ASC->GetTagCount(GateTag), 1);
		TestEqual(TEXT("authoritative Reset 保持 gate source map 为空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 0);
		TestFalse(TEXT("authoritative Reset 清除 NeedsReset"),
			FSigilAbilityEntitlementProjectionTestAccess::NeedsProjectionReset(*Context.ASC));
		TestNull(TEXT("authoritative Reset 清除投影 handle"),
			Context.ASC->FindAbilitySpecFromHandle(ProjectionHandle));
		TestEqual(TEXT("authoritative Reset 清空 runtime grant 账本"),
			FSigilAbilityEntitlementProjectionTestAccess::GetRuntimeGrantCount(*Context.ASC), 0);
		TestEqual(TEXT("authoritative Reset 清空 entitlement index"),
			FSigilAbilityEntitlementProjectionTestAccess::GetEntitlementCount(*Context.ASC), 0);
		TestEqual(TEXT("authoritative Reset 清空 projection handle 账本"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionHandleCount(*Context.ASC), 0);
		TestEqual(TEXT("authoritative Reset 清空 accepted revision"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedRevision(*Context.ASC), int64(INDEX_NONE));
		TestTrue(TEXT("authoritative Reset 清空 accepted digest"),
			FSigilAbilityEntitlementProjectionTestAccess::GetAcceptedDigest(*Context.ASC).IsEmpty());
		TestEqual(TEXT("authoritative Reset 推进 exclusive-tag fixture epoch"),
			FSigilAbilityEntitlementProjectionTestAccess::GetProjectionEpoch(*Context.ASC), EpochBeforeReset + 1);

		Context.ASC->RemoveLooseGameplayTag(GateTag, 1);
		TestEqual(TEXT("exclusive-tag fixture 清理外部贡献"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateTag), 0);
	}

	// GateB 的 tag delegate 若破坏另一 GateA，外层结束检查必须发现全 ledger 漂移并要求 Reset。
	{
		const FTestAscContext Context = CreateAsc(*World);
		const FGameplayTag GateA = GateTag;
		const FGameplayTag GateB = InputExactTag;
		const FName SourceA(TEXT("CrossGateSourceA"));
		const FName SourceB(TEXT("CrossGateSourceB"));
		TestTrue(TEXT("跨 Gate delegate fixture 使用两个独立 exact Tag"), GateA != GateB);

		Context.ASC->SetAbilityActivationGateSource(GateA, SourceA, true);
		TestTrue(TEXT("跨 Gate delegate 前 GateA SourceA 已入账"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateA, SourceA));
		TestEqual(TEXT("跨 Gate delegate 前 GateA source count=1"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateA), 1);
		TestEqual(TEXT("跨 Gate delegate 前 GateA explicit count=1"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateA), 1);
		TestEqual(TEXT("跨 Gate delegate 前只有 GateA ledger"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 1);

		int32 GateBEnableCallbackCount = 0;
		int32 GateBDisableCallbackCount = 0;
		int32 UnexpectedGateBCallbackCount = 0;
		bool bRemovedGateAFromGateBDelegate = false;
		bool bOnlyGateBObserved = true;
		const FDelegateHandle GateBDelegateHandle = Context.ASC->RegisterGameplayTagEvent(
			GateB, EGameplayTagEventType::NewOrRemoved).AddLambda(
			[&](const FGameplayTag ChangedTag, int32 NewCount)
			{
				bOnlyGateBObserved = bOnlyGateBObserved && ChangedTag == GateB;
				if (NewCount > 0)
				{
					++GateBEnableCallbackCount;
					if (!bRemovedGateAFromGateBDelegate)
					{
						bRemovedGateAFromGateBDelegate = true;
						Context.ASC->RemoveLooseGameplayTag(GateA, 1);
					}
					return;
				}
				if (NewCount == 0)
				{
					++GateBDisableCallbackCount;
					return;
				}
				++UnexpectedGateBCallbackCount;
			});

		AddExpectedErrorPlain(
			TEXT("SetAbilityActivationGateSource completed its requested gate bookkeeping but a tag delegate changed ASC context, tracked AbilitySpecs, colliding AbilitySpecs, or the expected gate count; an authoritative reset is required."),
			EAutomationExpectedErrorFlags::Exact,
			1);
		Context.ASC->SetAbilityActivationGateSource(GateB, SourceB, true);
		TestTrue(TEXT("GateB 首次 enable delegate 已移除 GateA loose tag"),
			bRemovedGateAFromGateBDelegate);
		TestEqual(TEXT("GateB 首次 enable delegate 只触发一次"), GateBEnableCallbackCount, 1);
		TestEqual(TEXT("GateB 外层结束前未触发 disable delegate"), GateBDisableCallbackCount, 0);
		TestEqual(TEXT("GateB delegate 没有异常 count"), UnexpectedGateBCallbackCount, 0);
		TestTrue(TEXT("GateB delegate 只观察 GateB"), bOnlyGateBObserved);
		TestTrue(TEXT("跨 Gate ledger 漂移置 NeedsReset"),
			FSigilAbilityEntitlementProjectionTestAccess::NeedsProjectionReset(*Context.ASC));
		TestEqual(TEXT("跨 Gate 漂移后保留 GateA/GateB 两个 ledger"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 2);
		TestTrue(TEXT("跨 Gate 漂移后 GateA map 仍有 SourceA"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateA, SourceA));
		TestTrue(TEXT("跨 Gate 漂移后 GateB map 已有 SourceB"),
			FSigilAbilityEntitlementProjectionTestAccess::HasGateSource(*Context.ASC, GateB, SourceB));
		TestEqual(TEXT("跨 Gate 漂移后 GateA source count=1"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateA), 1);
		TestEqual(TEXT("跨 Gate 漂移后 GateB source count=1"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateB), 1);
		TestEqual(TEXT("跨 Gate 漂移后 GateA explicit count=0"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateA), 0);
		TestEqual(TEXT("跨 Gate 漂移后 GateB explicit count=1"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateB), 1);

		Context.ASC->ResetAbilityEntitlementProjection();
		TestFalse(TEXT("跨 Gate authoritative Reset 清除 NeedsReset"),
			FSigilAbilityEntitlementProjectionTestAccess::NeedsProjectionReset(*Context.ASC));
		TestEqual(TEXT("跨 Gate authoritative Reset 清空 gate ledger"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*Context.ASC), 0);
		TestEqual(TEXT("跨 Gate authoritative Reset 清空 GateA source"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateA), 0);
		TestEqual(TEXT("跨 Gate authoritative Reset 清空 GateB source"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateSourceCount(*Context.ASC, GateB), 0);
		TestEqual(TEXT("跨 Gate authoritative Reset 后 GateA explicit count=0"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateA), 0);
		TestEqual(TEXT("跨 Gate authoritative Reset 后 GateB explicit count=0"),
			FSigilAbilityEntitlementProjectionTestAccess::GetExplicitTagCount(*Context.ASC, GateB), 0);
		TestEqual(TEXT("跨 Gate Reset 对 GateB 只触发一次 disable delegate"), GateBDisableCallbackCount, 1);
		TestEqual(TEXT("跨 Gate Reset 未重触发 GateB enable delegate"), GateBEnableCallbackCount, 1);
		TestTrue(TEXT("跨 Gate GateB delegate 可确定性注销"),
			Context.ASC->UnregisterGameplayTagEvent(
				GateBDelegateHandle, GateB, EGameplayTagEventType::NewOrRemoved));
	}

	// Pressed/Released 只匹配 exact DynamicSpecSourceTag；缺 ActorInfo 时 fail closed。
	{
		const FTestAscContext Context = CreateAsc(*World);
		FGameplayAbilitySpec ExactSpec(UGameplayAbility::StaticClass()->GetDefaultObject<UGameplayAbility>(), 1);
		ExactSpec.GetDynamicSpecSourceTags().AddTag(InputExactTag);
		const FGameplayAbilitySpecHandle ExactHandle = Context.ASC->GiveAbility(ExactSpec);
		FGameplayAbilitySpec OtherSpec(UGameplayAbility::StaticClass()->GetDefaultObject<UGameplayAbility>(), 1);
		OtherSpec.GetDynamicSpecSourceTags().AddTag(InputOtherTag);
		const FGameplayAbilitySpecHandle OtherHandle = Context.ASC->GiveAbility(OtherSpec);
		FGameplayAbilitySpec* ExactRuntimeSpec = Context.ASC->FindAbilitySpecFromHandle(ExactHandle);
		FGameplayAbilitySpec* OtherRuntimeSpec = Context.ASC->FindAbilitySpecFromHandle(OtherHandle);
		if (!TestNotNull(TEXT("exact input 目标 spec 可查"), ExactRuntimeSpec)
			|| !TestNotNull(TEXT("other input 目标 spec 可查"), OtherRuntimeSpec))
		{
			return false;
		}

		Context.ASC->AbilityInputTagPressed(InputParentTag);
		TestFalse(TEXT("父 Tag press 不命中 exact spec"), ExactRuntimeSpec->InputPressed);
		TestFalse(TEXT("父 Tag press 不命中 sibling spec"), OtherRuntimeSpec->InputPressed);
		TestFalse(TEXT("父 Tag press 不激活 exact spec"), ExactRuntimeSpec->IsActive());
		TestFalse(TEXT("父 Tag press 不激活 sibling spec"), OtherRuntimeSpec->IsActive());

		Context.ASC->AbilityInputTagPressed(InputExactTag);
		TestTrue(TEXT("exact Tag press 命中目标 spec"), ExactRuntimeSpec->InputPressed);
		TestFalse(TEXT("exact Tag press 不命中其他 spec"), OtherRuntimeSpec->InputPressed);
		TestTrue(TEXT("exact Tag press 激活目标 spec"), ExactRuntimeSpec->IsActive());
		TestFalse(TEXT("exact Tag press 不激活其他 spec"), OtherRuntimeSpec->IsActive());
		Context.ASC->AbilityInputTagPressed(InputOtherTag);
		TestTrue(TEXT("第二个 exact Tag press 命中其 spec"), OtherRuntimeSpec->InputPressed);
		TestTrue(TEXT("第二个 exact Tag press 激活其 spec"), OtherRuntimeSpec->IsActive());
		TestTrue(TEXT("第二个 exact Tag press 不误伤先前 active spec"), ExactRuntimeSpec->IsActive());

		Context.ASC->AbilityInputTagReleased(InputExactTag);
		TestFalse(TEXT("exact Tag release 释放目标 spec"), ExactRuntimeSpec->InputPressed);
		TestTrue(TEXT("exact Tag release 不释放其他 spec"), OtherRuntimeSpec->InputPressed);
		TestTrue(TEXT("exact Tag release 不误伤其他 active spec"), OtherRuntimeSpec->IsActive());
		Context.ASC->AbilityInputTagReleased(InputParentTag);
		TestTrue(TEXT("父 Tag release 不命中 child spec"), OtherRuntimeSpec->InputPressed);
		TestTrue(TEXT("父 Tag release 不误伤 exact active spec"), ExactRuntimeSpec->IsActive());
		TestTrue(TEXT("父 Tag release 不误伤 other active spec"), OtherRuntimeSpec->IsActive());
		Context.ASC->AbilityInputTagReleased(InputOtherTag);
		TestFalse(TEXT("第二个 exact Tag release 释放其 spec"), OtherRuntimeSpec->InputPressed);
		TestTrue(TEXT("第二个 exact Tag release 不误伤另一 active spec"), ExactRuntimeSpec->IsActive());

		USigilAbilitySystemComponent* MissingActorInfoASC = NewObject<USigilAbilitySystemComponent>(
			GetTransientPackage(), NAME_None, RF_Transient);
		TestStatus(*this, TEXT("缺 ActorInfo reconcile fail closed"),
			MissingActorInfoASC->ReconcileAbilityEntitlements(MakeSnapshot(0, {})),
			ESigilAbilityReconcileStatus::ActorInfoNotReady);
		MissingActorInfoASC->AbilityInputTagPressed(InputExactTag);
		MissingActorInfoASC->AbilityInputTagReleased(InputExactTag);
		MissingActorInfoASC->SetAbilityActivationGateSource(GateTag, FName(TEXT("MissingActorInfo")), true);
		TestEqual(TEXT("缺 ActorInfo 输入/gate 保持零状态"), MissingActorInfoASC->GetActivatableAbilities().Num(), 0);
		TestEqual(TEXT("缺 ActorInfo gate index 保持为空"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGateCount(*MissingActorInfoASC), 0);
	}

	// 旧 AbilitySet API 的默认等级保持兼容，静态包装器正确传递 OverrideLevel。
	{
		const FTestAscContext Context = CreateAsc(*World);
		USigilAbilitySet* LegacySet = CreateAbilitySet(TEXT("EntitlementLegacySet"));
		FSigilAbilityEntitlementProjectionTestAccess::AddAbility(
			*LegacySet, UGameplayAbility::StaticClass(), 3, 7, InputExactTag);

		FSigilAbilitySet_GrantedHandles DirectHandles;
		LegacySet->GiveToAbilitySystem(Context.ASC, &DirectHandles, nullptr);
		TestEqual(TEXT("旧实例 API 默认调用仍授予一份"),
			FSigilAbilityEntitlementProjectionTestAccess::GetGrantedHandleCount(DirectHandles), 1);
		FGameplayAbilitySpec* DirectSpec = Context.ASC->FindAbilitySpecFromHandle(
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstGrantedHandle(DirectHandles));
		TestNotNull(TEXT("旧实例 API spec 可查"), DirectSpec);
		if (DirectSpec)
		{
			TestEqual(TEXT("未 override 保留条目等级"), DirectSpec->Level, 3);
			TestEqual(TEXT("旧 API InputID 保持"), DirectSpec->InputID, 7);
			TestTrue(TEXT("旧 API dynamic tag 保持"), DirectSpec->GetDynamicSpecSourceTags().HasTagExact(InputExactTag));
		}
		USigilAbilitySet::TakeAbilitySetFromAbilitySystem(DirectHandles, Context.ASC);

		const TSoftObjectPtr<USigilAbilitySet> SoftLegacySet(LegacySet);
		FSigilAbilitySet_GrantedHandles DefaultStaticHandles =
			USigilAbilitySet::GiveAbilitySetToAbilitySystem(SoftLegacySet, Context.ASC, nullptr);
		FGameplayAbilitySpec* DefaultStaticSpec = Context.ASC->FindAbilitySpecFromHandle(
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstGrantedHandle(DefaultStaticHandles));
		TestNotNull(TEXT("旧三参数静态 API 保持可用"), DefaultStaticSpec);
		if (DefaultStaticSpec)
		{
			TestEqual(TEXT("旧三参数静态 API 保留条目等级"), DefaultStaticSpec->Level, 3);
		}
		USigilAbilitySet::TakeAbilitySetFromAbilitySystem(DefaultStaticHandles, Context.ASC);

		FSigilAbilitySet_GrantedHandles OverrideHandles =
			USigilAbilitySet::GiveAbilitySetToAbilitySystem(SoftLegacySet, Context.ASC, nullptr, 11);
		FGameplayAbilitySpec* OverrideSpec = Context.ASC->FindAbilitySpecFromHandle(
			FSigilAbilityEntitlementProjectionTestAccess::GetFirstGrantedHandle(OverrideHandles));
		TestNotNull(TEXT("静态包装 OverrideLevel spec 可查"), OverrideSpec);
		if (OverrideSpec)
		{
			TestEqual(TEXT("静态包装器传递 OverrideLevel"), OverrideSpec->Level, 11);
		}
		USigilAbilitySet::TakeAbilitySetFromAbilitySystem(OverrideHandles, Context.ASC);
	}

	WorldWrapper.ForwardErrorMessages(this);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
