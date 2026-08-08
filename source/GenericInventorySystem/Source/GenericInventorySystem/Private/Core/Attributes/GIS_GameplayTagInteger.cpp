// Copyright 2025 RedMoonGames All Rights Reserved.

#include "Attributes/GIS_GameplayTagInteger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GIS_GameplayTagInteger)

FString FGIS_GameplayTagInteger::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), Value);
}

void FGIS_GameplayTagIntegerContainer::AddItem(FGameplayTag Tag, int32 Value)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddItem"), ELogVerbosity::Warning);
		return;
	}

	if (Value > 0)
	{
		for (FGIS_GameplayTagInteger& Item : Items)
		{
			if (Item.Tag == Tag)
			{
				const int32 OldValue = Item.Value;
				const int32 NewValue = Item.Value + Value;
				Item.Value = NewValue;
				TagToValueMap[Tag] = NewValue;
				MarkItemDirty(Item);
				if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
				{
					Interface->OnTagIntegerUpdate(Tag, OldValue, NewValue);
				}
				return;
			}
		}

		FGIS_GameplayTagInteger& NewItem = Items.Emplace_GetRef(Tag, Value);
		TagToValueMap.Add(Tag, Value);
		MarkItemDirty(NewItem);
		if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
		{
			Interface->OnTagIntegerUpdate(Tag, 0, Value);
		}
	}
}

void FGIS_GameplayTagIntegerContainer::SetItem(FGameplayTag Tag, int32 Value)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to SetItem"), ELogVerbosity::Warning);
		return;
	}
	for (FGIS_GameplayTagInteger& Item : Items)
	{
		if (Item.Tag == Tag)
		{
			int32 OldValue = Item.Value;
			Item.Value = Value;
			TagToValueMap[Tag] = Value;
			MarkItemDirty(Item);
			if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
			{
				Interface->OnTagIntegerUpdate(Tag, OldValue, Value);
			}
			return;
		}
	}

	FGIS_GameplayTagInteger& NewItem = Items.Emplace_GetRef(Tag, Value);
	MarkItemDirty(NewItem);
	TagToValueMap.Add(Tag, Value);
	if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
	{
		Interface->OnTagIntegerUpdate(Tag, 0, Value);
	}
}

void FGIS_GameplayTagIntegerContainer::RemoveItem(FGameplayTag Tag, int32 Value)
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
			FGIS_GameplayTagInteger& Item = *It;
			if (Item.Tag == Tag)
			{
				if (Item.Value <= Value)
				{
					const int32 OldValue = Item.Value;
					It.RemoveCurrent();
					TagToValueMap.Remove(Tag);
					MarkArrayDirty();
					if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
					{
						Interface->OnTagIntegerUpdate(Tag, OldValue, 0);
					}
				}
				else
				{
					const int32 OldValue = Item.Value;
					const int32 NewValue = Item.Value - Value;
					Item.Value = NewValue;
					TagToValueMap[Tag] = NewValue;
					MarkItemDirty(Item);
					if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
					{
						Interface->OnTagIntegerUpdate(Tag, OldValue, NewValue);
					}
				}
				return;
			}
		}
	}
}

void FGIS_GameplayTagIntegerContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FGIS_GameplayTagInteger& Item = Items[Index];

		TagToValueMap.Remove(Item.Tag);
		if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
		{
			Interface->OnTagIntegerUpdate(Item.Tag, Item.PrevValue, 0);
		}
		Item.PrevValue = 0;
	}
}

void FGIS_GameplayTagIntegerContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FGIS_GameplayTagInteger& Item = Items[Index];
		TagToValueMap.Add(Item.Tag, Item.Value);
		if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
		{
			Interface->OnTagIntegerUpdate(Item.Tag, 0, Item.Value);
		}
		Item.PrevValue = Item.Value;
	}
}

void FGIS_GameplayTagIntegerContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FGIS_GameplayTagInteger& Item = Items[Index];
		TagToValueMap[Item.Tag] = Item.Value;
		if (IGIS_GameplayTagIntegerContainerOwner* Interface = Cast<IGIS_GameplayTagIntegerContainerOwner>(ContainerOwner))
		{
			Interface->OnTagIntegerUpdate(Item.Tag, Item.PrevValue, Item.Value);
		}
		Item.PrevValue = Item.Value;
	}
}
