// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Weapon/GCS_AttachmentRelationshipMapping.h"
#include "Animation/Skeleton.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ObjectSaveContext.h"

bool UGCS_AttachmentRelationshipMapping::FindRelationshipForMesh(const USkeletalMeshComponent* InSkeletalMeshComponent, const UStaticMesh* InStaticMesh, const USkeletalMesh* InSkeletalMesh,
                                                                 FName InSocketName, FGCS_AttachmentRelationship& OutRelationship) const
{
	bool bFoundMatchingSkeleton = false;
	if (bUseNameMatching)
	{
		if (!CompatibleSkeletonNames.IsEmpty() && InSkeletalMeshComponent && InSkeletalMeshComponent->GetSkeletalMeshAsset())
		{
			for (const FString& SkeletonName : CompatibleSkeletonNames)
			{
				if (SkeletonName.IsEmpty())
					continue;

				if (USkeleton* Skeleton = InSkeletalMeshComponent->GetSkeletalMeshAsset()->GetSkeleton())
				{
					if (Skeleton->GetName() == SkeletonName)
					{
						bFoundMatchingSkeleton = true;
						break;
					}
				}
			}
		}
	}
	else
	{
		if (!CompatibleSkeletons.IsEmpty() && InSkeletalMeshComponent && InSkeletalMeshComponent->GetSkeletalMeshAsset())
		{
			for (TSoftObjectPtr<USkeleton> CompatibleSkeleton : CompatibleSkeletons)
			{
				if (CompatibleSkeleton.IsNull())
					continue;

				if (!CompatibleSkeleton.IsValid())
				{
					CompatibleSkeleton.LoadSynchronous();
				}
				if (CompatibleSkeleton == InSkeletalMeshComponent->GetSkeletalMeshAsset()->GetSkeleton())
				{
					bFoundMatchingSkeleton = true;
					break;
				}
			}
		}
	}


	if (!bFoundMatchingSkeleton)
	{
		return false;
	}

	for (const FGCS_AttachmentRelationship& Rel : Relationships)
	{
		if (Rel.SocketName != InSocketName)
		{
			continue;
		}

		if (IsValid(InStaticMesh) && Rel.StaticMesh.IsValid() && InStaticMesh == Rel.StaticMesh.Get())
		{
			OutRelationship = Rel;
			return true;
		}

		if (IsValid(InSkeletalMesh) && Rel.SkeletalMesh.IsValid() && InSkeletalMesh == Rel.SkeletalMesh.Get())
		{
			OutRelationship = Rel;
			return true;
		}
	}
	return false;
}

#if WITH_EDITOR

void UGCS_AttachmentRelationshipMapping::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FGCS_AttachmentRelationship& Relationship : Relationships)
	{
		if (Relationship.SocketName == NAME_None)
		{
			Relationship.EditorFriendlyName = "Invalid setup";
		}
		else
		{
			Relationship.EditorFriendlyName = FString::Format(TEXT("Adjust SM({0})/SKM({1}) for Socket({2})"),
			                                                  {
				                                                  Relationship.StaticMesh.IsNull() ? TEXT("None") : Relationship.StaticMesh.LoadSynchronous()->GetName(),
				                                                  Relationship.SkeletalMesh.IsNull() ? TEXT("None") : Relationship.SkeletalMesh.LoadSynchronous()->GetName(),
				                                                  Relationship.SocketName.ToString()
			                                                  });
		}
	}
	Super::PreSave(SaveContext);
}

EDataValidationResult UGCS_AttachmentRelationshipMapping::IsDataValid(class FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif
