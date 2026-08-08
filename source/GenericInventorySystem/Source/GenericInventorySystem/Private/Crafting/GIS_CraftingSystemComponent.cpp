// Copyright 2025 RedMoonGames All Rights Reserved.


#include "GIS_CraftingSystemComponent.h"
#include "Engine/World.h"
#include "GIS_InventorySystemComponent.h"
#include "GIS_CurrencySystemComponent.h"
#include "GIS_InventoryFunctionLibrary.h"
#include "GIS_InventorySubsystem.h"
#include "GIS_ItemCollection.h"
#include "Items/GIS_ItemDefinition.h"
#include "GIS_ItemFragment_CraftingRecipe.h"
#include "Items/GIS_ItemInstance.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_CraftingSystemComponent)

// Sets default values for this component's properties
UGIS_CraftingSystemComponent::UGIS_CraftingSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	NumOfSelectedItemsCache = 0;
	// ...
}

bool UGIS_CraftingSystemComponent::Craft(const UGIS_ItemDefinition* Recipe, UGIS_InventorySystemComponent* CostInventory, int32 Quantity)
{
	const UGIS_ItemFragment_CraftingRecipe* RecipeFragment = GetRecipeFragment(Recipe);
	if (RecipeFragment == nullptr)
	{
		return false;
	}

	return CraftInternal(Recipe, CostInventory, Quantity);
}

bool UGIS_CraftingSystemComponent::CanCraft(const UGIS_ItemDefinition* RecipeDefinition, UGIS_InventorySystemComponent* Inventory, int32 Quantity)
{
	return CanCraftInternal(RecipeDefinition, Inventory, Quantity);
}

bool UGIS_CraftingSystemComponent::RemoveItemIngredients(UGIS_InventorySystemComponent* Inventory, const TArray<FGIS_ItemInfo>& ItemIngredients)
{
	for (int32 i = 0; i < ItemIngredients.Num(); i++)
	{
		FGIS_ItemInfo ItemInfoToRemove;
		ItemInfoToRemove.Item = ItemIngredients[i].Item;
		ItemInfoToRemove.Amount = ItemIngredients[i].Amount;
		if (Inventory->GetDefaultCollection()->RemoveItem(ItemInfoToRemove).Amount == ItemInfoToRemove.Amount) { continue; }
		return false;
	}
	return false;
}

bool UGIS_CraftingSystemComponent::IsValidRecipe_Implementation(const UGIS_ItemDefinition* RecipeDefinition) const
{
	if (const UGIS_ItemFragment_CraftingRecipe* Fragment = GetRecipeFragment(RecipeDefinition))
	{
		return !Fragment->InputItems.IsEmpty() && !Fragment->OutputItems.IsEmpty();
	}
	return false;
}

bool UGIS_CraftingSystemComponent::CanCraftInternal(const UGIS_ItemDefinition* RecipeDefinition, UGIS_InventorySystemComponent* Inventory,
                                                    int32 Quantity)
{
	if (!IsValidRecipe(RecipeDefinition))
	{
		return false;
	}

	const UGIS_ItemFragment_CraftingRecipe* RecipeFragment = GetRecipeFragment(RecipeDefinition);
	check(RecipeFragment != nullptr);

	if (!SelectItemForIngredients(Inventory, RecipeFragment->InputItems, Quantity))
	{
		return false;
	}

	TArray<FGIS_ItemInfo> Selected;
	int32 SelectedCount = 0;
	// Has enough items? 有足够的道具？
	if (!CheckIfEnoughItemIngredients(RecipeFragment->InputItems, Quantity, SelectedItemsCache, Selected, SelectedCount))
	{
		return false;
	}

	UGIS_CurrencySystemComponent* CurrencySystem = Inventory->GetCurrencySystem();
	if (CurrencySystem == nullptr)
	{
		return false;
	}

	// Has enough currencies? 有足够货币？
	if (!CurrencySystem->HasCurrencies(UGIS_InventoryFunctionLibrary::MultiplyCurrencies(RecipeFragment->InputCurrencies, Quantity)))
	{
		return false;
	}

	return true;
}

bool UGIS_CraftingSystemComponent::CraftInternal(const UGIS_ItemDefinition* RecipeDefinition, UGIS_InventorySystemComponent* Inventory, int32 Quantity)
{
	const UGIS_ItemFragment_CraftingRecipe* RecipeFragment = GetRecipeFragment(RecipeDefinition);
	if (RecipeFragment == nullptr)
	{
		return false;
	}

	if (!SelectItemForIngredients(Inventory, RecipeFragment->InputItems, Quantity))
	{
		return false;
	}

	if (!CanCraftInternal(RecipeDefinition, Inventory, Quantity))
	{
		return false;
	}

	UGIS_CurrencySystemComponent* CurrencySystem = Inventory->GetCurrencySystem();

	bool bCurrencyRemoveSuccess = CurrencySystem->RemoveCurrencies(UGIS_InventoryFunctionLibrary::MultiplyCurrencies(RecipeFragment->InputCurrencies, Quantity));

	bool bItemRemoveSuccess = RemoveItemIngredients(Inventory, SelectedItemsCache);

	if (bCurrencyRemoveSuccess && bItemRemoveSuccess)
	{
		ProduceCraftingOutput(RecipeDefinition, Inventory, Quantity);
		return true;
	}
	return false;
}

void UGIS_CraftingSystemComponent::ProduceCraftingOutput(const UGIS_ItemDefinition* RecipeDefinition, UGIS_InventorySystemComponent* Inventory, int32 Quantity)
{
	const UGIS_ItemFragment_CraftingRecipe* RecipeFragment = GetRecipeFragment(RecipeDefinition);
	if (RecipeFragment == nullptr)
	{
		return;
	}

	TArray<FGIS_ItemDefinitionAmount> ItemAmounts = UGIS_InventoryFunctionLibrary::MultiplyItemAmounts(RecipeFragment->OutputItems, Quantity);

	for (int32 i = 0; i < ItemAmounts.Num(); i++)
	{
		FGIS_ItemInfo ItemInfoToAdd;
		ItemInfoToAdd.Item = UGIS_InventorySubsystem::Get(GetWorld())->CreateItem(Inventory->GetOwner(), ItemAmounts[i].Definition);
		ItemInfoToAdd.Amount = ItemAmounts[i].Amount;
		Inventory->AddItem(ItemInfoToAdd);
	}
}

const UGIS_ItemFragment_CraftingRecipe* UGIS_CraftingSystemComponent::GetRecipeFragment(const UGIS_ItemDefinition* RecipeDefinition) const
{
	return RecipeDefinition ? RecipeDefinition->FindFragment<UGIS_ItemFragment_CraftingRecipe>() : nullptr;
}

bool UGIS_CraftingSystemComponent::SelectItemForIngredients(const UGIS_InventorySystemComponent* Inventory, const TArray<FGIS_ItemDefinitionAmount>& ItemIngredients, int32 Quantity)
{
	SelectedItemsCache.Empty();
	NumOfSelectedItemsCache = 0;
	for (int32 i = 0; i < ItemIngredients.Num(); i++)
	{
		auto RequiredItem = ItemIngredients[i];
		int32 NeededAmount = RequiredItem.Amount * Quantity;

		TArray<FGIS_ItemInfo> FilteredItemInfos;
		if (Inventory->GetItemInfosByDefinition(RequiredItem.Definition, FilteredItemInfos))
		{
			for (int32 j = 0; j < FilteredItemInfos.Num(); j++)
			{
				const FGIS_ItemInfo& itemInfo = FilteredItemInfos[j];

				int32 HaveAmount = itemInfo.Amount;

				bool FoundSelectedItem = false;
				for (int k = 0; k < NumOfSelectedItemsCache; k++)
				{
					const FGIS_ItemInfo& SelectedItemInfo = SelectedItemsCache[k];
					if (itemInfo.Item != SelectedItemInfo.Item) { continue; }
					if (itemInfo.StackId != SelectedItemInfo.StackId) { continue; }

					FoundSelectedItem = true;

					HaveAmount = FMath::Max(0, HaveAmount - SelectedItemInfo.Amount);
					int32 additionalIgnoreAmount = FMath::Clamp(HaveAmount, 0, NeededAmount);
					//更新已经选择道具信息。
					SelectedItemsCache[k] = FGIS_ItemInfo(SelectedItemInfo.Amount + additionalIgnoreAmount, SelectedItemInfo);
				}

				int32 SelectedAmount = FMath::Clamp(HaveAmount, 0, NeededAmount);

				//记录已选择道具信息。
				if (FoundSelectedItem == false)
				{
					SelectedItemsCache.SetNum(NumOfSelectedItemsCache + 1);
					SelectedItemsCache[NumOfSelectedItemsCache] = FGIS_ItemInfo(SelectedAmount, itemInfo);
					NumOfSelectedItemsCache++;
				}

				NeededAmount -= SelectedAmount;
				if (NeededAmount <= 0) { break; }
			}
		}
		if (NeededAmount <= 0) { continue; }
		return false;
	}
	return true;
}

bool UGIS_CraftingSystemComponent::CheckIfEnoughItemIngredients(const TArray<FGIS_ItemDefinitionAmount>& ItemIngredients, int32 Quantity, const TArray<FGIS_ItemInfo>& SelectedItems,
                                                                TArray<FGIS_ItemInfo>& ItemsToIgnore, int32& NumOfItemsToIgnore)
{
	for (int32 i = 0; i < ItemIngredients.Num(); i++)
	{
		const FGIS_ItemDefinitionAmount& ingredientAmount = ItemIngredients[i];

		int32 neededAmount = Quantity * ingredientAmount.Amount;

		for (int32 j = 0; j < SelectedItems.Num(); j++)
		{
			const FGIS_ItemInfo& itemInfo = SelectedItems[j];
			if (ingredientAmount.Definition != itemInfo.Item->GetDefinition()) { continue; }

			int32 haveAmount = itemInfo.Amount;
			bool foundMatch = false;


			int32 selectedAmount = FMath::Clamp(haveAmount, 0, neededAmount);

			if (foundMatch == false)
			{
				ItemsToIgnore.SetNum(NumOfItemsToIgnore + 1);
				ItemsToIgnore[NumOfItemsToIgnore] = FGIS_ItemInfo(selectedAmount, itemInfo);
				NumOfItemsToIgnore++;
			}

			neededAmount -= selectedAmount;
			if (neededAmount <= 0) { break; }
		}

		if (neededAmount > 0)
		{
			return false;
		}
	}

	return true;
}
