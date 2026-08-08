// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GGA_AttributeGroupNameCustomization.h"
#include "DetailWidgetRow.h"
#include "GGA_AbilitySystemGlobals.h"
#include "GGA_AbilitySystemStructLibrary.h"
#include "PropertyCustomizationHelpers.h"


TSharedRef<IPropertyTypeCustomization> FGGA_AttributeGroupNameCustomization::MakeInstance()
{
	return MakeShareable(new FGGA_AttributeGroupNameCustomization());
}

void FGGA_AttributeGroupNameCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const UGGA_AbilitySystemGlobals* Globals = Cast<UGGA_AbilitySystemGlobals>(IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals());

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

			TArray<FString> RowParts; //[0]GroupName [1]SetName [2]AttribueName
			RowName.ParseIntoArray(RowParts, TEXT("."));
			if (RowParts.Num() != 3)
			{
				continue;
			}

			TArray<FString> GroupParts; //[0]MainName [1]SubName
			RowName.ParseIntoArray(GroupParts, TEXT("->"));
			if (GroupParts.Num() != 2)
			{
				//Add class name as group.
				FNameMap.FindOrAdd(FName(*RowParts[0]));
			}
			else if (GroupParts.Num() == 2)
			{
				TArray<FName>& Rows = FNameMap.FindOrAdd(FName(*GroupParts[0]));
				Rows.AddUnique(FName(*GroupParts[1]));
			}
		}
	}
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	MainNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGGA_AttributeGroupName, MainName));
	SubNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGGA_AttributeGroupName, SubName));
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
						                                                   FOnGetPropertyComboBoxStrings::CreateStatic(&FGGA_AttributeGroupNameCustomization::GeneratePrimaryComboboxStrings, true,
						                                                                                               false, &FNameMap),
						                                                   FOnGetPropertyComboBoxValue::CreateSP(this, &FGGA_AttributeGroupNameCustomization::GenerateMainString),
						                                                   FOnPropertyComboBoxValueSelected::CreateSP(this, &FGGA_AttributeGroupNameCustomization::OnMainValueSelected))
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.Padding(2.f, 0.f, 2.f, 0.f)
					[
						PropertyCustomizationHelpers::MakePropertyComboBox(MainNamePropertyHandle,
						                                                   FOnGetPropertyComboBoxStrings::CreateStatic(&FGGA_AttributeGroupNameCustomization::GenerateSubComboboxStrings, true,
						                                                                                               false, &FNameMap,
						                                                                                               MainNamePropertyHandle),
						                                                   FOnGetPropertyComboBoxValue::CreateSP(this, &FGGA_AttributeGroupNameCustomization::GenerateSubString),
						                                                   FOnPropertyComboBoxValueSelected::CreateSP(this, &FGGA_AttributeGroupNameCustomization::OnSubValueSelected))
					]
				]

			];
	}
}


void FGGA_AttributeGroupNameCustomization::GeneratePrimaryComboboxStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems,
                                                                          bool bAllowClear, bool bAllowAll,
                                                                          TMap<FName, TArray<FName>>* InItems)
{
	for (auto Iter = InItems->CreateConstIterator(); Iter; ++Iter)
	{
		OutComboBoxStrings.Add(MakeShared<FString>(Iter.Key().ToString()));
	}
}

void FGGA_AttributeGroupNameCustomization::GenerateSubComboboxStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips,
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

FString FGGA_AttributeGroupNameCustomization::GenerateSubString()
{
	void* TagDataPtr = nullptr;
	SubNamePropertyHandle->GetValueData(TagDataPtr);
	const FName* TagPtr = static_cast<FName*>(TagDataPtr);

	FString TagString = TagPtr ? TagPtr->ToString() : "Invalid";

	return TagString;
}

void FGGA_AttributeGroupNameCustomization::OnSubValueSelected(const FString& String)
{
	SubNamePropertyHandle->SetValue(FName(*String));
}

FString FGGA_AttributeGroupNameCustomization::GenerateMainString()
{
	void* TagDataPtr = nullptr;
	MainNamePropertyHandle->GetValueData(TagDataPtr);
	const FName* TagPtr = static_cast<FName*>(TagDataPtr);

	FString TagString = TagPtr ? TagPtr->ToString() : "Invalid";

	return TagString;
}

void FGGA_AttributeGroupNameCustomization::OnMainValueSelected(const FString& String)
{
	MainNamePropertyHandle->SetValue(FName(*String));
	SubNamePropertyHandle->ResetToDefault();
}

void FGGA_AttributeGroupNameCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
                                                             IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}
