// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilSaveSubsystem.h"

#include "JsonUtils/RapidJsonUtils.h"
#include "Kismet/GameplayStatics.h"
#include "SigilJsonSaveGame.h"

namespace
{
bool IsStrictJson(const FString& JsonText)
{
	UE::Json::FDocument Document;
#if PLATFORM_LITTLE_ENDIAN
	using FSourceEncoding = rapidjson::UTF16LE<TCHAR>;
#else
	using FSourceEncoding = rapidjson::UTF16BE<TCHAR>;
#endif
	Document.Parse<rapidjson::kParseDefaultFlags, FSourceEncoding>(*JsonText, JsonText.Len());
	return !Document.HasParseError();
}
}

bool USigilSaveSubsystem::SaveJson(const FString& SlotName, const FString& JsonText, const int32 UserIndex) const
{
	if (!IsStrictJson(JsonText))
	{
		return false;
	}

	USigilJsonSaveGame* SaveGame = NewObject<USigilJsonSaveGame>();
	SaveGame->JsonText = JsonText;
	return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
}

bool USigilSaveSubsystem::LoadJson(const FString& SlotName, FString& OutJsonText, const int32 UserIndex) const
{
	OutJsonText.Reset();

	const USaveGame* LoadedObject = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	const USigilJsonSaveGame* LoadedSave = Cast<USigilJsonSaveGame>(LoadedObject);
	if (!LoadedSave || !IsStrictJson(LoadedSave->JsonText))
	{
		return false;
	}

	OutJsonText = LoadedSave->JsonText;
	return true;
}

bool USigilSaveSubsystem::DeleteJson(const FString& SlotName, const int32 UserIndex) const
{
	if (SlotName.IsEmpty())
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}
