// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilSocketRelationshipMapping.generated.h"

/**
 * Structure for socket adjustments.
 * 插槽调整结构。
 */
USTRUCT(BlueprintType)
struct SIGILINTERACTION_API FSigilSocketAdjustment
{
	GENERATED_BODY()

	/**
	 * Array of skeleton names for the adjustment.
	 * 调整适用的骨骼名称数组。
	 */
	UPROPERTY(EditAnywhere, Category="GGS")
	TArray<FString> ForSkeletons;

	/**
	 * Name of the socket.
	 * 插槽名称。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS")
	FName SocketName{NAME_None};

	/**
	 * Relative transform for the socket.
	 * 插槽的相对变换。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS")
	FTransform RelativeTransform;

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-friendly name for the adjustment.
	 * 调整的编辑器友好名称。
	 */
	UPROPERTY(EditAnywhere, Category="GGS", meta=(EditCondition=false, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

/**
 * Structure for socket relationships.
 * 插槽关系结构。
 */
USTRUCT(BlueprintType)
struct SIGILINTERACTION_API FSigilSocketRelationship
{
	GENERATED_BODY()

	/**
	 * Mesh asset associated with the relationship.
	 * 与关系关联的网格资产。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS")
	TSoftObjectPtr<UStreamableRenderAsset> MeshAsset;

	/**
	 * Array of socket adjustments for the mesh.
	 * 网格的插槽调整数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilSocketAdjustment> Adjustments;

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-friendly name for the relationship.
	 * 关系的编辑器友好名称。
	 */
	UPROPERTY(EditAnywhere, Category="GGS", meta=(EditCondition=false, EditConditionHides))
	FString EditorFriendlyName;
#endif
};

/**
 * Data asset for defining socket relationships for mesh attachments.
 * 定义网格附件插槽关系的数据资产。
 */
UCLASS(BlueprintType)
class SIGILINTERACTION_API USigilSocketRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Finds a socket adjustment for a given mesh and socket.
	 * 查找给定网格和插槽的插槽调整。
	 * @param InParentMeshComponent The parent mesh component. 父网格组件。
	 * @param InMeshAsset The mesh asset. 网格资产。
	 * @param InSocketName The socket name. 插槽名称。
	 * @param OutAdjustment The found socket adjustment (output). 找到的插槽调整（输出）。
	 * @return True if an adjustment was found, false otherwise. 如果找到调整返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=False, Category="GGS|Utilities")
	bool FindSocketAdjustment(const USkeletalMeshComponent* InParentMeshComponent, const UStreamableRenderAsset* InMeshAsset, FName InSocketName,
	                          FSigilSocketAdjustment& OutAdjustment) const;

	/**
	 * Finds a socket adjustment across multiple mappings.
	 * 在多个映射中查找插槽调整。
	 * @param InMappings The socket relationship mappings. 插槽关系映射。
	 * @param InParentMeshComponent The parent mesh component. 父网格组件。
	 * @param InMeshAsset The mesh asset. 网格资产。
	 * @param InSocketName The socket name. 插槽名称。
	 * @param OutAdjustment The found socket adjustment (output). 找到的插槽调整（输出）。
	 * @return True if an adjustment was found, false otherwise. 如果找到调整返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category="GGS|Utilities")
	static bool FindSocketAdjustmentInMappings(TArray<TSoftObjectPtr<USigilSocketRelationshipMapping>> InMappings, const USkeletalMeshComponent* InParentMeshComponent,
	                                           const UStreamableRenderAsset* InMeshAsset, FName InSocketName,
	                                           FSigilSocketAdjustment& OutAdjustment);

	/**
	 * Array of socket relationships.
	 * 插槽关系数组。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GGS", meta=(TitleProperty="EditorFriendlyName"))
	TArray<FSigilSocketRelationship> Relationships;

#if WITH_EDITORONLY_DATA
	/**
	 * Pre-save processing for editor.
	 * 编辑器预保存处理。
	 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};