// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Utilities/SigilSocketRelationshipMapping.h"
#include "Engine/StreamableRenderAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/Skeleton.h"
#include "UObject/ObjectSaveContext.h"

bool USigilSocketRelationshipMapping::FindSocketAdjustment(const USkeletalMeshComponent* InParentMeshComponent, const UStreamableRenderAsset* InMeshAsset, FName InSocketName,
                                                          FSigilSocketAdjustment& OutAdjustment) const
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

	for (const FSigilSocketRelationship& Relationship : Relationships)
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
			const FSigilSocketAdjustment& Adjustment = Relationship.Adjustments[i];
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

bool USigilSocketRelationshipMapping::FindSocketAdjustmentInMappings(TArray<TSoftObjectPtr<USigilSocketRelationshipMapping>> InMappings, const USkeletalMeshComponent* InParentMeshComponent,
                                                                    const UStreamableRenderAsset* InMeshAsset, FName InSocketName,
                                                                    FSigilSocketAdjustment& OutAdjustment)
{
	for (TSoftObjectPtr<USigilSocketRelationshipMapping> Mapping : InMappings)
	{
		if (Mapping.IsNull())
		{
			continue;
		}
		if (const USigilSocketRelationshipMapping* LoadedMapping = Mapping.LoadSynchronous())
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
void USigilSocketRelationshipMapping::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FSigilSocketRelationship& Relationship : Relationships)
	{
		if (Relationship.MeshAsset.IsNull())
		{
			Relationship.EditorFriendlyName = TEXT("Invalid!");
		}
		else
		{
			UStreamableRenderAsset* MeshAsset = Relationship.MeshAsset.LoadSynchronous();
			Relationship.EditorFriendlyName = MeshAsset->GetName();
			for (FSigilSocketAdjustment& Adjustment : Relationship.Adjustments)
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
