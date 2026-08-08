// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilDetailSectionsBuilder.generated.h"

class USigilListEntryDetailSection;
class USigilListEntry;

/**
 * Base class for customizing how detail sections are gathered for an object.
 * 自定义如何为对象收集细节部分的基类。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Generic UI"))
class SIGILUI_API USigilDetailSectionsBuilder : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Gathers detail section classes for the specified data.
	 * 为指定数据收集细节部分类。
	 * @param Data The data object. 数据对象。
	 * @return Array of detail section classes. 细节部分类数组。
	 */
	UFUNCTION(Blueprintable, BlueprintNativeEvent)
	TArray<TSoftClassPtr<USigilListEntryDetailSection>> GatherDetailSections(const UObject* Data);
	virtual TArray<TSoftClassPtr<USigilListEntryDetailSection>> GatherDetailSections_Implementation(const UObject* Data);
};

/**
 * Structure for mapping object classes to detail section classes.
 * 将对象类映射到细节部分类的结构。
 */
USTRUCT()
struct SIGILUI_API FSigilEntryDetailsClassSections
{
	GENERATED_BODY()

	/**
	 * Array of detail section classes for an object class.
	 * 对象类的细节部分类数组。
	 */
	UPROPERTY(EditAnywhere, Category="GUIS")
	TArray<TSoftClassPtr<USigilListEntryDetailSection>> Sections;
};

/**
 * Concrete class for mapping object classes to detail sections.
 * 将对象类映射到细节部分的实体类。
 */
UCLASS(NotBlueprintable)
class SIGILUI_API USigilDetailSectionBuilder_Class : public USigilDetailSectionsBuilder
{
	GENERATED_BODY()

public:
	/**
	 * Gathers detail section classes for the specified data.
	 * 为指定数据收集细节部分类。
	 * @param Data The data object. 数据对象。
	 * @return Array of detail section classes. 细节部分类数组。
	 */
	virtual TArray<TSoftClassPtr<USigilListEntryDetailSection>> GatherDetailSections_Implementation(const UObject* Data) override;

protected:
	/**
	 * Mapping of object classes to their detail section configurations.
	 * 对象类到其细节部分配置的映射。
	 */
	UPROPERTY(EditDefaultsOnly, Category="GUIS", meta = (AllowAbstract))
	TMap<TSubclassOf<UObject>, FSigilEntryDetailsClassSections> SectionsForClasses;
};
