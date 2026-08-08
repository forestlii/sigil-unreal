// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Attributes/GIS_GameplayTagFloat.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_GameplayTagFloat)


FString FGIS_GameplayTagFloat::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%f"), *Tag.ToString(), Value);
}

void FGIS_GameplayTagFloatContainer::AddItem(FGameplayTag Tag, float Value)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddItem"), ELogVerbosity::Warning);
		return;
	}

	if (Value > 0)
	{
		for (FGIS_GameplayTagFloat& Item : Items)
		{
			// handle adding to existing value.
			if (Item.Tag == Tag)
			{
				const float OldValue = Item.Value;
				const float NewValue = Item.Value + Value;
				Item.Value = NewValue;
				TagToValueMap[Tag] = NewValue;
				MarkItemDirty(Item);
				if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
				{
					Interface->OnTagFloatUpdate(Tag, OldValue, NewValue);
				}
				return;
			}
		}

		// handle adding new item.
		FGIS_GameplayTagFloat& NewItem = Items.Emplace_GetRef(Tag, Value);
		TagToValueMap.Add(Tag, Value);
		MarkItemDirty(NewItem);
		if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
		{
			Interface->OnTagFloatUpdate(Tag, 0, Value);
		}
	}
}

void FGIS_GameplayTagFloatContainer::SetItem(FGameplayTag Tag, float Value)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to SetItem"), ELogVerbosity::Warning);
		return;
	}
	for (FGIS_GameplayTagFloat& Item : Items)
	{
		if (Item.Tag == Tag)
		{
			const float OldValue = Item.Value;
			Item.Value = Value;
			TagToValueMap[Tag] = Value;
			MarkItemDirty(Item);
			if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
			{
				Interface->OnTagFloatUpdate(Tag, OldValue, Value);
			}
			return;
		}
	}

	FGIS_GameplayTagFloat& NewItem = Items.Emplace_GetRef(Tag, Value);
	MarkItemDirty(NewItem);
	TagToValueMap.Add(Tag, Value);
	if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
	{
		Interface->OnTagFloatUpdate(Tag, 0, Value);
	}
}

void FGIS_GameplayTagFloatContainer::SetItems(const TArray<FGIS_GameplayTagFloat>& NewItems)
{
	Items = NewItems;
	TagToValueMap.Empty();
	for (const FGIS_GameplayTagFloat& NewItem : NewItems)
	{
		TagToValueMap.Add(NewItem.Tag, NewItem.Value);
	}
	MarkArrayDirty();
}

void FGIS_GameplayTagFloatContainer::EmptyItems()
{
	Items.Empty();
	TagToValueMap.Empty();
	MarkArrayDirty();
}

void FGIS_GameplayTagFloatContainer::RemoveItem(FGameplayTag Tag, float Value)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveItem"), ELogVerbosity::Warning);
		return;
	}

	//@TODO: Should we error if you try to remove a Item that doesn't exist or has a smaller count?
	if (Value > 0)
	{
		for (auto It = Items.CreateIterator(); It; ++It)
		{
			FGIS_GameplayTagFloat& Item = *It;
			if (Item.Tag == Tag)
			{
				if (Item.Value <= Value)
				{
					const float OldValue = Item.Value;
					It.RemoveCurrent();
					TagToValueMap.Remove(Tag);
					MarkArrayDirty();
					if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
					{
						Interface->OnTagFloatUpdate(Tag, OldValue, 0);
					}
				}
				else
				{
					const float OldValue = Item.Value;
					const float NewValue = Item.Value - Value;
					Item.Value = NewValue;
					TagToValueMap[Tag] = NewValue;
					MarkItemDirty(Item);
					if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
					{
						Interface->OnTagFloatUpdate(Tag, OldValue, NewValue);
					}
				}
				return;
			}
		}
	}
}

void FGIS_GameplayTagFloatContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FGIS_GameplayTagFloat& Item = Items[Index];
		TagToValueMap.Remove(Item.Tag);
		if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
		{
			Interface->OnTagFloatUpdate(Item.Tag, Item.PrevValue, 0);
		}
		Item.PrevValue = 0;
	}
}

void FGIS_GameplayTagFloatContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FGIS_GameplayTagFloat& Item = Items[Index];
		TagToValueMap.Add(Item.Tag, Item.Value);
		if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
		{
			Interface->OnTagFloatUpdate(Item.Tag, 0, Item.Value);
		}
		Item.PrevValue = Item.Value;
	}
}

void FGIS_GameplayTagFloatContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FGIS_GameplayTagFloat& Item = Items[Index];
		TagToValueMap[Item.Tag] = Item.Value;
		if (IGIS_GameplayTagFloatContainerOwner* Interface = Cast<IGIS_GameplayTagFloatContainerOwner>(ContainerOwner))
		{
			Interface->OnTagFloatUpdate(Item.Tag, Item.PrevValue, Item.Value);
		}
		Item.PrevValue = Item.Value;
	}
}
