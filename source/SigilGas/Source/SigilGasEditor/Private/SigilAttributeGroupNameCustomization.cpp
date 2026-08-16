// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "SigilAttributeGroupNameCustomization.h"
#include "DetailWidgetRow.h"
#include "SigilAbilitySystemGlobals.h"
#include "SigilAbilitySystemStructLibrary.h"
#include "PropertyCustomizationHelpers.h"


TSharedRef<IPropertyTypeCustomization> FSigilAttributeGroupNameCustomization::MakeInstance()
{
	return MakeShareable(new FSigilAttributeGroupNameCustomization());
}

void FSigilAttributeGroupNameCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const USigilAbilitySystemGlobals* Globals = Cast<USigilAbilitySystemGlobals>(IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals());

	if (Globals == nullptr)
	{
		return;
	}

	FNameMap.Reset();

	for (const UCurveTable* CurTable : Globals->GetAttributeDefaultsTables())
	{
		if (!IsValid(CurTable))
		{
			continue;
		}

		for (const TPair<FName, FRealCurve*>& CurveRow : CurTable->GetRowMap())
		{
			FString RowName = CurveRow.Key.ToString();

			// Row layout mirrors the engine's FAttributeSetInitter: `Group.SetName.Attribute`, split on '.'.
			TArray<FString> RowParts; //[0]GroupName [1]SetName [2]AttributeName
			RowName.ParseIntoArray(RowParts, TEXT("."));
			if (RowParts.Num() != 3)
			{
				continue;
			}

			// Only the group segment may carry a sub-group, encoded as `Main->Sub` (see FSigilAttributeGroupName::SubNameSeparator).
			TArray<FString> GroupParts; //[0]MainName [1]SubName
			RowParts[0].ParseIntoArray(GroupParts, FSigilAttributeGroupName::SubNameSeparator);
			if (GroupParts.Num() == 2)
			{
				TArray<FName>& Rows = FNameMap.FindOrAdd(FName(*GroupParts[0]));
				Rows.AddUnique(FName(*GroupParts[1]));
			}
			else
			{
				FNameMap.FindOrAdd(FName(*RowParts[0]));
			}
		}
	}
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	MainNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSigilAttributeGroupName, MainName));
	SubNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSigilAttributeGroupName, SubName));
	if (MainNamePropertyHandle.IsValid() && SubNamePropertyHandle.IsValid())
	{
		HeaderRow
			.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			.MinDesiredWidth(600)
			.MaxDesiredWidth(4096)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.Padding(1.f, 0.f, 2.f, 0.f)
					[
						PropertyCustomizationHelpers::MakePropertyComboBox(MainNamePropertyHandle,
						                                                   FOnGetPropertyComboBoxStrings::CreateStatic(&FSigilAttributeGroupNameCustomization::GeneratePrimaryComboboxStrings, true,
						                                                                                               false, &FNameMap),
						                                                   FOnGetPropertyComboBoxValue::CreateSP(this, &FSigilAttributeGroupNameCustomization::GenerateMainString),
						                                                   FOnPropertyComboBoxValueSelected::CreateSP(this, &FSigilAttributeGroupNameCustomization::OnMainValueSelected))
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.Padding(2.f, 0.f, 2.f, 0.f)
					[
						PropertyCustomizationHelpers::MakePropertyComboBox(MainNamePropertyHandle,
						                                                   FOnGetPropertyComboBoxStrings::CreateStatic(&FSigilAttributeGroupNameCustomization::GenerateSubComboboxStrings, true,
						                                                                                               false, &FNameMap,
						                                                                                               MainNamePropertyHandle),
						                                                   FOnGetPropertyComboBoxValue::CreateSP(this, &FSigilAttributeGroupNameCustomization::GenerateSubString),
						                                                   FOnPropertyComboBoxValueSelected::CreateSP(this, &FSigilAttributeGroupNameCustomization::OnSubValueSelected))
					]
				]

			];
	}
}


void FSigilAttributeGroupNameCustomization::GeneratePrimaryComboboxStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems,
                                                                          bool bAllowClear, bool bAllowAll,
                                                                          TMap<FName, TArray<FName>>* InItems)
{
	for (auto Iter = InItems->CreateConstIterator(); Iter; ++Iter)
	{
		OutComboBoxStrings.Add(MakeShared<FString>(Iter.Key().ToString()));
	}
}

void FSigilAttributeGroupNameCustomization::GenerateSubComboboxStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips,
                                                                      TArray<bool>& OutRestrictedItems, bool bAllowClear, bool bAllowAll,
                                                                      TMap<FName, TArray<FName>>* InItems, TSharedPtr<IPropertyHandle> PrimaryKey)
{
	void* TagDataPtr = nullptr;
	PrimaryKey->GetValueData(TagDataPtr);
	const FName* TagPtr = static_cast<FName*>(TagDataPtr);

	if (!TagPtr || TagPtr->IsNone())
	{
		OutComboBoxStrings.Add(MakeShared<FString>("Invalid"));
	}
	else
	{
		const auto Arr = InItems->Find(*TagPtr);
		if (Arr)
		{
			for (auto& Name : *Arr)
			{
				OutComboBoxStrings.Add(MakeShared<FString>(Name.ToString()));
			}
		}
	}
}

FString FSigilAttributeGroupNameCustomization::GenerateSubString()
{
	void* TagDataPtr = nullptr;
	SubNamePropertyHandle->GetValueData(TagDataPtr);
	const FName* TagPtr = static_cast<FName*>(TagDataPtr);

	FString TagString = TagPtr ? TagPtr->ToString() : "Invalid";

	return TagString;
}

void FSigilAttributeGroupNameCustomization::OnSubValueSelected(const FString& String)
{
	SubNamePropertyHandle->SetValue(FName(*String));
}

FString FSigilAttributeGroupNameCustomization::GenerateMainString()
{
	void* TagDataPtr = nullptr;
	MainNamePropertyHandle->GetValueData(TagDataPtr);
	const FName* TagPtr = static_cast<FName*>(TagDataPtr);

	FString TagString = TagPtr ? TagPtr->ToString() : "Invalid";

	return TagString;
}

void FSigilAttributeGroupNameCustomization::OnMainValueSelected(const FString& String)
{
	MainNamePropertyHandle->SetValue(FName(*String));
	SubNamePropertyHandle->ResetToDefault();
}

void FSigilAttributeGroupNameCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
                                                             IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}
