// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilDialogueAsset.generated.h"

class USigilNarrativeCondition;
class USigilNarrativeEvent;

UENUM(BlueprintType)
enum class ESigilDialogueNodeType : uint8
{
	Line,
	Choice,
	End
};

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilDialogueOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName OptionId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName TargetNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeCondition>> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Sigil|Narrative")
	TArray<TObjectPtr<USigilNarrativeEvent>> Events;
};

USTRUCT(BlueprintType)
struct SIGILNARRATIVE_API FSigilDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	ESigilDialogueNodeType NodeType = ESigilDialogueNodeType::Line;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName SpeakerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName NextNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<FSigilDialogueOption> Options;
};

UCLASS(BlueprintType)
class SIGILNARRATIVE_API USigilDialogueAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName DialogueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	FName EntryNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative")
	TArray<FSigilDialogueNode> Nodes;

	UFUNCTION(BlueprintCallable, Category = "Sigil|Narrative")
	bool ValidateDefinition(FText& OutError) const;

	const FSigilDialogueNode* FindNode(FName NodeId) const;
};
