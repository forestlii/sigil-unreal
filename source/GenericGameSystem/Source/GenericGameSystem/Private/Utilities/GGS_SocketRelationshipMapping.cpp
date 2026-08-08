// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Utilities/GGS_SocketRelationshipMapping.h"
#include "Engine/StreamableRenderAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/Skeleton.h"
#include "UObject/ObjectSaveContext.h"

bool UGGS_SocketRelationshipMapping::FindSocketAdjustment(const USkeletalMeshComponent* InParentMeshComponent, const UStreamableRenderAsset* InMeshAsset, FName InSocketName,
                                                          FGGS_SocketAdjustment& OutAdjustment) const
{
	if (InParentMeshComponent == nullptr || InMeshAsset == nullptr || InSocketName.IsNone())
	{
		return false;
	}
	USkeleton* Skeleton = InParentMeshComponent->GetSkeletalMeshAsset()->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}
	FString SkeletonName = Skeleton->GetName();

	for (const FGGS_SocketRelationship& Relationship : Relationships)
	{
		UStreamableRenderAsset* Key{nullptr};
		if (!Relationship.MeshAsset.IsNull())
		{
			Key = Relationship.MeshAsset.LoadSynchronous();
		}
		if (!Key || Key->GetName() != InMeshAsset->GetName())
		{
			continue;
		}
		for (int32 i = Relationship.Adjustments.Num() - 1; i >= 0; i--)
		{
			const FGGS_SocketAdjustment& Adjustment = Relationship.Adjustments[i];
			bool bMatchSkeleton = Adjustment.ForSkeletons.IsEmpty() ? true : Adjustment.ForSkeletons.Contains(SkeletonName);
			if (bMatchSkeleton && Adjustment.SocketName == InSocketName)
			{
				OutAdjustment = Adjustment;
				return true;
			}
		}
	}

	return false;
}

bool UGGS_SocketRelationshipMapping::FindSocketAdjustmentInMappings(TArray<TSoftObjectPtr<UGGS_SocketRelationshipMapping>> InMappings, const USkeletalMeshComponent* InParentMeshComponent,
                                                                    const UStreamableRenderAsset* InMeshAsset, FName InSocketName,
                                                                    FGGS_SocketAdjustment& OutAdjustment)
{
	for (TSoftObjectPtr<UGGS_SocketRelationshipMapping> Mapping : InMappings)
	{
		if (Mapping.IsNull())
		{
			continue;
		}
		if (const UGGS_SocketRelationshipMapping* LoadedMapping = Mapping.LoadSynchronous())
		{
			if (LoadedMapping->FindSocketAdjustment(InParentMeshComponent, InMeshAsset, InSocketName, OutAdjustment))
			{
				return true;
			}
		}
	}
	return false;
}

#if WITH_EDITORONLY_DATA
void UGGS_SocketRelationshipMapping::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FGGS_SocketRelationship& Relationship : Relationships)
	{
		if (Relationship.MeshAsset.IsNull())
		{
			Relationship.EditorFriendlyName = TEXT("Invalid!");
		}
		else
		{
			UStreamableRenderAsset* MeshAsset = Relationship.MeshAsset.LoadSynchronous();
			Relationship.EditorFriendlyName = MeshAsset->GetName();
			for (FGGS_SocketAdjustment& Adjustment : Relationship.Adjustments)
			{
				if (Adjustment.SocketName == NAME_None)
				{
					Adjustment.EditorFriendlyName = "Empty adjustments!";
				}
				if (Adjustment.ForSkeletons.IsEmpty())
				{
					Adjustment.EditorFriendlyName = Adjustment.SocketName.ToString();
				}
				else
				{
					FString SkeletonNames;
					for (const FString& ForSkeleton : Adjustment.ForSkeletons)
					{
						SkeletonNames = SkeletonNames.Append(ForSkeleton);
					}
					Adjustment.EditorFriendlyName = FString::Format(TEXT("{0} on {1}"), {Adjustment.SocketName.ToString(), SkeletonNames});
				}
			}
		}
	}
	Super::PreSave(SaveContext);
}
#endif
