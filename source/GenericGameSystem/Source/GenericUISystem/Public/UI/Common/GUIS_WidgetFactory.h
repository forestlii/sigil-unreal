// Copyright 2025 RedMoonGames All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DataAsset.h"
#include "GUIS_WidgetFactory.generated.h"

class UUserWidget;

/**
 * Base class for selecting appropriate widget classes for objects.
 * 为对象选择合适小部件类的基类。
 */
UCLASS(Abstract, Blueprintable, BlueprintType, HideDropdown, Const)
class GENERICUISYSTEM_API UGUIS_WidgetFactory : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor.
	 * 默认构造函数。
	 */
	UGUIS_WidgetFactory();

	/**
	 * Finds the appropriate widget class for the given data.
	 * 为给定数据查找合适的小部件类。
	 * @param Data The data object. 数据对象。
	 * @return The widget class. 小部件类。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GUIS")
	TSubclassOf<UUserWidget> FindWidgetClassForData(const UObject *Data) const;

protected:
	/**
	 * Validates the data for the widget factory.
	 * 验证小部件工厂的数据。
	 * @param ValidationMessage The validation message (output). 验证消息（输出）。
	 * @return True if valid, false otherwise. 如果有效返回true，否则返回false。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="GUIS")
	bool OnDataValidation(FText &ValidationMessage) const;
	virtual bool OnDataValidation_Implementation(FText &ValidationMessage) const;

#if WITH_EDITOR
	/**
	 * Validates data in the editor.
	 * 在编辑器中验证数据。
	 * @param Context The data validation context. 数据验证上下文。
	 * @return The validation result. 验证结果。
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext &Context) const override;
#endif
};