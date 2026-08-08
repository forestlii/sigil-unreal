// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GCS_AttachmentRelationshipMapping.generated.h"

class USkeleton;
class USkeletalMesh;
class UStaticMesh;
class USkeletalMeshComponent;

/**
 * Deprecated!! Use SocketRelationshipMapping from GGS!
 * 弃用了，使用GGS中的SocketRelationshipMapping。
 */
USTRUCT(BlueprintType)
struct FGCS_AttachmentRelationship
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS")
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS")
	FName SocketName{NAME_None};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS")
	FTransform RelativeTransform;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="GGS", meta=(EditCondition=false, EditConditionHides))
	FString EditorFriendlyName;
#endif
};


/**
 * Deprecated!! Use SocketRelationshipMapping from GGS!
 * 弃用了，使用GGS中的SocketRelationshipMapping。
 */
UCLASS(BlueprintType, Const)
class GENERICCOMBATSYSTEM_API UGCS_AttachmentRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * @param InSkeletalMeshComponent The parent skeletal mesh component that need to be attached to. 
	 * @param InStaticMesh The static mesh you want to attach.
	 * @param InSkeletalMesh The skeletal mesh you want to attach.
	 * @param InSocketName  The socket name you want to attach.
	 * @param OutRelationship The result attachment relationship.
	 * @return true if any matching found.
	 */
	UFUNCTION(BlueprintCallable,BlueprintPure=False, Category="GGS|Utilities",meta=(DeprecatedFunction,DeprecationMessage="Use SocketRelationshipMapping from GGS!"))
	bool FindRelationshipForMesh(UPARAM(meta=(DisplayName="In Parent Mesh")) const USkeletalMeshComponent* InSkeletalMeshComponent, const UStaticMesh* InStaticMesh, const USkeletalMesh* InSkeletalMesh, FName InSocketName,
	                             FGCS_AttachmentRelationship& OutRelationship) const;

	/**
	 * Will restrict this mapping to CompatibleSkeletons, or no restriction if left empty.
	 */
	UPROPERTY(EditAnywhere, Category="GGS")
	TArray<TSoftObjectPtr<USkeleton>> CompatibleSkeletons;

	UPROPERTY(EditAnywhere, Category="GGS")
	TArray<FString> CompatibleSkeletonNames;

	UPROPERTY(EditAnywhere, Category="GGS")
	bool bUseNameMatching{true};	
	
	UPROPERTY(EditAnywhere, Category="GGS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FGCS_AttachmentRelationship> Relationships;


#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
