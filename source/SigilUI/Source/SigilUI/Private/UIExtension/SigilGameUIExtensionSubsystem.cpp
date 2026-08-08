// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "UIExtension/SigilGameUIExtensionSubsystem.h"

#include "SigilUILogChannels.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilGameUIExtensionSubsystem)

class FSubsystemCollectionBase;

//=========================================================

void FSigilGameUIExtPointHandle::Unregister()
{
	if (USigilExtensionSubsystem* ExtensionSourcePtr = ExtensionSource.Get())
	{
		ExtensionSourcePtr->UnregisterExtensionPoint(*this);
		ExtensionSource = nullptr;
		DataPtr.Reset();
	}
}

//=========================================================

FSigilGameUIExtHandle::FSigilGameUIExtHandle()
{
}

FSigilGameUIExtHandle::FSigilGameUIExtHandle(USigilExtensionSubsystem* InExtensionSource, const TSharedPtr<FSigilGameUIExt>& InDataPtr)
{
	ExtensionSource = InExtensionSource;
	DataPtr = InDataPtr;
}

void FSigilGameUIExtHandle::Unregister()
{
	if (USigilExtensionSubsystem* ExtensionSourcePtr = ExtensionSource.Get())
	{
		ExtensionSourcePtr->UnregisterExtension(*this);
		ExtensionSource = nullptr;
		DataPtr.Reset();
	}
}

//=========================================================

bool FSigilGameUIExtPoint::DoesExtensionPassContract(const FSigilGameUIExt* Extension) const
{
	if (UObject* DataPtr = Extension->Data)
	{
		const bool bMatchesContext =
			(ContextObject.IsExplicitlyNull() && Extension->ContextObject.IsExplicitlyNull()) ||
			ContextObject == Extension->ContextObject;

		// Make sure the contexts match.
		if (bMatchesContext)
		{
			// The data can either be the literal class of the data type, or a instance of the class type.
			const UClass* DataClass = DataPtr->IsA(UClass::StaticClass()) ? Cast<UClass>(DataPtr) : DataPtr->GetClass();
			for (const UClass* AllowedDataClass : AllowedDataClasses)
			{
				if (DataClass->IsChildOf(AllowedDataClass) || DataClass->ImplementsInterface(AllowedDataClass))
				{
					return true;
				}
			}
		}
	}

	return false;
}

FSigilGameUIExtPointHandle::FSigilGameUIExtPointHandle()
{
}

FSigilGameUIExtPointHandle::FSigilGameUIExtPointHandle(USigilExtensionSubsystem* InExtensionSource, const TSharedPtr<FSigilGameUIExtPoint>& InDataPtr)
{
	ExtensionSource = InExtensionSource;
	DataPtr = InDataPtr;
}

//=========================================================

void USigilExtensionSubsystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);
	if (USigilExtensionSubsystem* ExtensionSubsystem = Cast<USigilExtensionSubsystem>(InThis))
	{
		for (auto MapIt = ExtensionSubsystem->ExtensionPointMap.CreateIterator(); MapIt; ++MapIt)
		{
			for (const TSharedPtr<FSigilGameUIExtPoint>& ValueElement : MapIt.Value())
			{
				Collector.AddReferencedObjects(ValueElement->AllowedDataClasses);
			}
		}

		for (auto MapIt = ExtensionSubsystem->ExtensionMap.CreateIterator(); MapIt; ++MapIt)
		{
			for (const TSharedPtr<FSigilGameUIExt>& ValueElement : MapIt.Value())
			{
				Collector.AddReferencedObject(ValueElement->Data);
			}
		}
	}
}

void USigilExtensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USigilExtensionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FSigilGameUIExtPointHandle USigilExtensionSubsystem::RegisterExtensionPoint(const FGameplayTag& ExtensionPointTag, ESigilGameUIExtPointMatchType ExtensionPointTagMatchType,
                                                                            const TArray<UClass*>& AllowedDataClasses, FExtendExtensionPointDelegate ExtensionCallback)
{
	return RegisterExtensionPointForContext(ExtensionPointTag, nullptr, ExtensionPointTagMatchType, AllowedDataClasses, ExtensionCallback);
}

FSigilGameUIExtPointHandle USigilExtensionSubsystem::RegisterExtensionPointForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject,
                                                                                      ESigilGameUIExtPointMatchType ExtensionPointTagMatchType,
                                                                                      const TArray<UClass*>& AllowedDataClasses, FExtendExtensionPointDelegate ExtensionCallback)
{
	if (!ExtensionPointTag.IsValid())
	{
		UE_LOG(LogSigilUI_Extension, Warning, TEXT("Trying to register an invalid extension point."));
		return FSigilGameUIExtPointHandle();
	}

	if (!ExtensionCallback.IsBound())
	{
		UE_LOG(LogSigilUI_Extension, Warning, TEXT("Trying to register an invalid extension point."));
		return FSigilGameUIExtPointHandle();
	}

	if (AllowedDataClasses.Num() == 0)
	{
		UE_LOG(LogSigilUI_Extension, Warning, TEXT("Trying to register an invalid extension point."));
		return FSigilGameUIExtPointHandle();
	}

	FExtensionPointList& List = ExtensionPointMap.FindOrAdd(ExtensionPointTag);

	TSharedPtr<FSigilGameUIExtPoint>& Entry = List.Add_GetRef(MakeShared<FSigilGameUIExtPoint>());
	Entry->ExtensionPointTag = ExtensionPointTag;
	Entry->ContextObject = ContextObject;
	Entry->ExtensionPointTagMatchType = ExtensionPointTagMatchType;
	Entry->AllowedDataClasses = AllowedDataClasses;
	Entry->Callback = MoveTemp(ExtensionCallback);

	UE_LOG(LogSigilUI_Extension, Verbose, TEXT("Extension Point [%s] Registered"), *ExtensionPointTag.ToString());

	NotifyExtensionPointOfExtensions(Entry);

	return FSigilGameUIExtPointHandle(this, Entry);
}

FSigilGameUIExtHandle USigilExtensionSubsystem::RegisterExtensionAsWidget(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority)
{
	return RegisterExtensionAsData(ExtensionPointTag, nullptr, WidgetClass, Priority);
}

FSigilGameUIExtHandle USigilExtensionSubsystem::RegisterExtensionAsWidgetForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority)
{
	return RegisterExtensionAsData(ExtensionPointTag, ContextObject, WidgetClass, Priority);
}

FSigilGameUIExtHandle USigilExtensionSubsystem::RegisterExtensionAsData(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority)
{
	if (!ExtensionPointTag.IsValid())
	{
		UE_LOG(LogSigilUI_Extension, Warning, TEXT("Trying to register an invalid extension."));
		return FSigilGameUIExtHandle();
	}

	if (!Data)
	{
		UE_LOG(LogSigilUI_Extension, Warning, TEXT("Trying to register an invalid extension."));
		return FSigilGameUIExtHandle();
	}

	FExtensionList& List = ExtensionMap.FindOrAdd(ExtensionPointTag);

	TSharedPtr<FSigilGameUIExt>& Entry = List.Add_GetRef(MakeShared<FSigilGameUIExt>());
	Entry->ExtensionPointTag = ExtensionPointTag;
	Entry->ContextObject = ContextObject;
	Entry->Data = Data;
	Entry->Priority = Priority;

	if (ContextObject)
	{
		UE_LOG(LogSigilUI_Extension, Verbose, TEXT("Extension [%s] @ [%s] Registered"), *GetNameSafe(Data), *ExtensionPointTag.ToString());
	}
	else
	{
		UE_LOG(LogSigilUI_Extension, Verbose, TEXT("Extension [%s] for [%s] @ [%s] Registered"), *GetNameSafe(Data), *GetNameSafe(ContextObject), *ExtensionPointTag.ToString());
	}

	NotifyExtensionPointsOfExtension(ESigilGameUIExtAction::Added, Entry);

	return FSigilGameUIExtHandle(this, Entry);
}

void USigilExtensionSubsystem::NotifyExtensionPointOfExtensions(TSharedPtr<FSigilGameUIExtPoint>& ExtensionPoint)
{
	for (FGameplayTag Tag = ExtensionPoint->ExtensionPointTag; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FExtensionList* ListPtr = ExtensionMap.Find(Tag))
		{
			// Copy in case there are removals while handling callbacks
			FExtensionList ExtensionArray(*ListPtr);

			for (const TSharedPtr<FSigilGameUIExt>& Extension : ExtensionArray)
			{
				if (ExtensionPoint->DoesExtensionPassContract(Extension.Get()))
				{
					FSigilGameUIExtRequest Request = CreateExtensionRequest(Extension);
					ExtensionPoint->Callback.ExecuteIfBound(ESigilGameUIExtAction::Added, Request);
				}
			}
		}

		if (ExtensionPoint->ExtensionPointTagMatchType == ESigilGameUIExtPointMatchType::ExactMatch)
		{
			break;
		}
	}
}

void USigilExtensionSubsystem::NotifyExtensionPointsOfExtension(ESigilGameUIExtAction Action, TSharedPtr<FSigilGameUIExt>& Extension)
{
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Extension->ExtensionPointTag; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FExtensionPointList* ListPtr = ExtensionPointMap.Find(Tag))
		{
			// Copy in case there are removals while handling callbacks
			FExtensionPointList ExtensionPointArray(*ListPtr);

			for (const TSharedPtr<FSigilGameUIExtPoint>& ExtensionPoint : ExtensionPointArray)
			{
				if (bOnInitialTag || (ExtensionPoint->ExtensionPointTagMatchType == ESigilGameUIExtPointMatchType::PartialMatch))
				{
					if (ExtensionPoint->DoesExtensionPassContract(Extension.Get()))
					{
						FSigilGameUIExtRequest Request = CreateExtensionRequest(Extension);
						ExtensionPoint->Callback.ExecuteIfBound(Action, Request);
					}
				}
			}
		}

		bOnInitialTag = false;
	}
}

void USigilExtensionSubsystem::UnregisterExtension(const FSigilGameUIExtHandle& ExtensionHandle)
{
	if (ExtensionHandle.IsValid())
	{
		checkf(ExtensionHandle.ExtensionSource == this, TEXT("Trying to unregister an extension that's not from this extension subsystem."));

		TSharedPtr<FSigilGameUIExt> Extension = ExtensionHandle.DataPtr;
		if (FExtensionList* ListPtr = ExtensionMap.Find(Extension->ExtensionPointTag))
		{
			if (Extension->ContextObject.IsExplicitlyNull())
			{
				UE_LOG(LogSigilUI_Extension, Verbose, TEXT("Extension [%s] @ [%s] Unregistered"), *GetNameSafe(Extension->Data), *Extension->ExtensionPointTag.ToString());
			}
			else
			{
				UE_LOG(LogSigilUI_Extension, Verbose, TEXT("Extension [%s] for [%s] @ [%s] Unregistered"), *GetNameSafe(Extension->Data), *GetNameSafe(Extension->ContextObject.Get()),
				       *Extension->ExtensionPointTag.ToString());
			}

			NotifyExtensionPointsOfExtension(ESigilGameUIExtAction::Removed, Extension);

			ListPtr->RemoveSwap(Extension);

			if (ListPtr->Num() == 0)
			{
				ExtensionMap.Remove(Extension->ExtensionPointTag);
			}
		}
	}
	else
	{
		UE_LOG(LogSigilUI_Extension, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

void USigilExtensionSubsystem::UnregisterExtensionPoint(const FSigilGameUIExtPointHandle& ExtensionPointHandle)
{
	if (ExtensionPointHandle.IsValid())
	{
		check(ExtensionPointHandle.ExtensionSource == this);

		const TSharedPtr<FSigilGameUIExtPoint> ExtensionPoint = ExtensionPointHandle.DataPtr;
		if (FExtensionPointList* ListPtr = ExtensionPointMap.Find(ExtensionPoint->ExtensionPointTag))
		{
			UE_LOG(LogSigilUI_Extension, Verbose, TEXT("Extension Point [%s] Unregistered"), *ExtensionPoint->ExtensionPointTag.ToString());

			ListPtr->RemoveSwap(ExtensionPoint);
			if (ListPtr->Num() == 0)
			{
				ExtensionPointMap.Remove(ExtensionPoint->ExtensionPointTag);
			}
		}
	}
	else
	{
		UE_LOG(LogSigilUI_Extension, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

FSigilGameUIExtRequest USigilExtensionSubsystem::CreateExtensionRequest(const TSharedPtr<FSigilGameUIExt>& Extension)
{
	FSigilGameUIExtRequest Request;
	Request.ExtensionHandle = FSigilGameUIExtHandle(this, Extension);
	Request.ExtensionPointTag = Extension->ExtensionPointTag;
	Request.Priority = Extension->Priority;
	Request.Data = Extension->Data;
	Request.ContextObject = Extension->ContextObject.Get();

	return Request;
}

FSigilGameUIExtPointHandle USigilExtensionSubsystem::K2_RegisterExtensionPoint(FGameplayTag ExtensionPointTag, ESigilGameUIExtPointMatchType ExtensionPointTagMatchType,
                                                                               const TArray<TSoftClassPtr<UClass>>& AllowedDataClasses,
                                                                               FExtendExtensionPointDynamicDelegate ExtensionCallback)
{
	TArray<UClass*> LoadedClasses;

	for (const TSoftClassPtr<UClass>& DataClass : AllowedDataClasses)
	{
		if (!DataClass.IsNull())
		{
			LoadedClasses.Add(DataClass.LoadSynchronous());
		}
	}
	return RegisterExtensionPoint(ExtensionPointTag, ExtensionPointTagMatchType, LoadedClasses, FExtendExtensionPointDelegate::CreateWeakLambda(
		                              ExtensionCallback.GetUObject(), [this, ExtensionCallback](ESigilGameUIExtAction Action, const FSigilGameUIExtRequest& Request)
		                              {
			                              ExtensionCallback.ExecuteIfBound(Action, Request);
		                              }));
}

FSigilGameUIExtHandle USigilExtensionSubsystem::K2_RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSoftClassPtr<UUserWidget> WidgetClass, int32 Priority)
{
	if (!WidgetClass.IsNull())
	{
		return RegisterExtensionAsWidget(ExtensionPointTag, WidgetClass.LoadSynchronous(), Priority);
	}
	return FSigilGameUIExtHandle();
}

FSigilGameUIExtHandle USigilExtensionSubsystem::K2_RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSoftClassPtr<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority)
{
	if (ContextObject && !WidgetClass.IsNull())
	{
		return RegisterExtensionAsWidgetForContext(ExtensionPointTag, ContextObject, WidgetClass.LoadSynchronous(), Priority);
	}
	FFrame::KismetExecutionMessage(TEXT("A null ContextObject was passed to Register Extension (Widget For Context)"), ELogVerbosity::Error);
	return FSigilGameUIExtHandle();
}

FSigilGameUIExtHandle USigilExtensionSubsystem::K2_RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority)
{
	return RegisterExtensionAsData(ExtensionPointTag, nullptr, Data, Priority);
}

FSigilGameUIExtHandle USigilExtensionSubsystem::K2_RegisterExtensionAsDataForContext(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority)
{
	if (ContextObject)
	{
		return RegisterExtensionAsData(ExtensionPointTag, ContextObject, Data, Priority);
	}
	FFrame::KismetExecutionMessage(TEXT("A null ContextObject was passed to Register Extension (Data For Context)"), ELogVerbosity::Error);
	return FSigilGameUIExtHandle();
}

//=========================================================

USigilExtensionFunctionLibrary::USigilExtensionFunctionLibrary()
{
}

void USigilExtensionFunctionLibrary::UnregisterExtension(FSigilGameUIExtHandle& Handle)
{
	Handle.Unregister();
}

bool USigilExtensionFunctionLibrary::IsValidExtension(FSigilGameUIExtHandle& Handle)
{
	return Handle.IsValid();
}

//=========================================================

void USigilExtensionFunctionLibrary::UnregisterExtensionPoint(FSigilGameUIExtPointHandle& Handle)
{
	Handle.Unregister();
}

bool USigilExtensionFunctionLibrary::IsValidExtensionPoint(FSigilGameUIExtPointHandle& Handle)
{
	return Handle.IsValid();
}
