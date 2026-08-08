// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Layout/Visibility.h"
#include "IPropertyTypeCustomization.h"

class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IPropertyHandle;

/** Details customization for FAttributeBasedFloat */
class SIGILGASEDITOR_API FSigilAttributeGroupNameCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** Overridden to provide the property name or hide, if necessary */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	static void GeneratePrimaryComboboxStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems, bool bAllowClear, bool bAllowAll,
											   TMap<FName, TArray<FName>>* InItems);
	FString GenerateMainString();
	void OnMainValueSelected(const FString& String);
	static void GenerateSubComboboxStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems, bool bAllowClear, bool bAllowAll,
												 TMap<FName, TArray<FName>>* InItems, TSharedPtr<IPropertyHandle> PrimaryKey);
	FString GenerateSubString();
	void OnSubValueSelected(const FString& String);
	/** Overridden to allow for possibly being hidden */
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	TMap<FName, TArray<FName>> FNameMap;

	TSharedPtr<IPropertyHandle> MainNamePropertyHandle;
	TSharedPtr<IPropertyHandle> SubNamePropertyHandle;
	TWeakPtr<IPropertyUtilities> PropertyUtilities;
};
