// Copyright (c) 2026 Likeon. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "SigilJsonSaveGame.h"
#include "SigilSaveSubsystem.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilSaveJsonRoundTripTest,
	"SigilSave.Json.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilSaveRejectsMalformedJsonTest,
	"SigilSave.Json.RejectsMalformedInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilSaveRejectsMalformedStoredJsonTest,
	"SigilSave.Json.RejectsMalformedStoredData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSigilSaveRejectsEmptySlotNameTest,
	"SigilSave.Json.RejectsEmptySlotName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSigilSaveJsonRoundTripTest::RunTest(const FString& Parameters)
{
	constexpr int32 UserIndex = 0;
	const FString SlotName = TEXT("SigilSave_Automation_JsonRoundTrip");
	const FString ExpectedJson = TEXT("{\"player\":\"Likeon\",\"level\":7}");

	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilSaveSubsystem* Subsystem = NewObject<USigilSaveSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem should be constructible"), Subsystem);
	if (!Subsystem)
	{
		return false;
	}

	Subsystem->DeleteJson(SlotName, UserIndex);

	FString LoadedJson = TEXT("stale");
	TestFalse(TEXT("A missing slot should not load"), Subsystem->LoadJson(SlotName, LoadedJson, UserIndex));
	TestTrue(TEXT("Saving valid JSON should succeed"), Subsystem->SaveJson(SlotName, ExpectedJson, UserIndex));
	TestTrue(TEXT("The saved slot should load"), Subsystem->LoadJson(SlotName, LoadedJson, UserIndex));
	TestEqual(TEXT("Loaded JSON should match the saved text exactly"), LoadedJson, ExpectedJson);
	TestTrue(TEXT("Deleting the saved slot should succeed"), Subsystem->DeleteJson(SlotName, UserIndex));
	TestFalse(TEXT("The deleted slot should no longer load"), Subsystem->LoadJson(SlotName, LoadedJson, UserIndex));

	return true;
}

bool FSigilSaveRejectsMalformedJsonTest::RunTest(const FString& Parameters)
{
	constexpr int32 UserIndex = 0;
	const FString SlotName = TEXT("SigilSave_Automation_InvalidJson");
	const FString TrailingCommaSlotName = TEXT("SigilSave_Automation_TrailingCommaJson");

	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilSaveSubsystem* Subsystem = NewObject<USigilSaveSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem should be constructible"), Subsystem);
	if (!Subsystem)
	{
		return false;
	}

	Subsystem->DeleteJson(SlotName, UserIndex);
	Subsystem->DeleteJson(TrailingCommaSlotName, UserIndex);
	TestTrue(TEXT("Valid JSON control should save"), Subsystem->SaveJson(SlotName, TEXT("{\"value\":1}"), UserIndex));
	TestTrue(TEXT("Valid JSON control should delete"), Subsystem->DeleteJson(SlotName, UserIndex));
	TestFalse(TEXT("Malformed JSON should be rejected"), Subsystem->SaveJson(SlotName, TEXT("not-json"), UserIndex));
	TestFalse(TEXT("JSON with a trailing comma should be rejected"), Subsystem->SaveJson(TrailingCommaSlotName, TEXT("{\"value\":1,}"), UserIndex));

	FString LoadedJson;
	TestFalse(TEXT("Rejected JSON should not create a save"), Subsystem->LoadJson(SlotName, LoadedJson, UserIndex));
	TestFalse(TEXT("Rejected trailing-comma JSON should not create a save"), Subsystem->LoadJson(TrailingCommaSlotName, LoadedJson, UserIndex));
	return true;
}

bool FSigilSaveRejectsMalformedStoredJsonTest::RunTest(const FString& Parameters)
{
	constexpr int32 UserIndex = 0;
	const FString SlotName = TEXT("SigilSave_Automation_InvalidStoredJson");

	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilSaveSubsystem* Subsystem = NewObject<USigilSaveSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem should be constructible"), Subsystem);
	if (!Subsystem)
	{
		return false;
	}

	Subsystem->DeleteJson(SlotName, UserIndex);
	USigilJsonSaveGame* InvalidSave = NewObject<USigilJsonSaveGame>();
	InvalidSave->JsonText = TEXT("{\"value\":1,}");
	TestTrue(TEXT("Test fixture should write malformed stored JSON"), UGameplayStatics::SaveGameToSlot(InvalidSave, SlotName, UserIndex));

	FString LoadedJson = TEXT("stale");
	TestFalse(TEXT("Loading malformed stored JSON should fail"), Subsystem->LoadJson(SlotName, LoadedJson, UserIndex));
	TestTrue(TEXT("A failed load should clear the output"), LoadedJson.IsEmpty());
	TestTrue(TEXT("Malformed test data should be deleted"), Subsystem->DeleteJson(SlotName, UserIndex));
	return true;
}

bool FSigilSaveRejectsEmptySlotNameTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	USigilSaveSubsystem* Subsystem = NewObject<USigilSaveSubsystem>(GameInstance);
	TestNotNull(TEXT("Subsystem should be constructible"), Subsystem);
	if (!Subsystem)
	{
		return false;
	}

	TestFalse(TEXT("Deleting an empty slot name should be rejected"), Subsystem->DeleteJson(TEXT(""), 0));
	return true;
}

#endif
