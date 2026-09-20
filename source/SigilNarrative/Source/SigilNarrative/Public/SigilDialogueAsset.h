// Copyright (c) 2026 Likeon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SigilDialogueAsset.generated.h"

class USigilNarrativeCondition;
class USigilNarrativeEvent;
class USoundBase;

UENUM(BlueprintType)
enum class ESigilDialogueNodeType : uint8
{
	Line,
	Choice,
	End
};

/**
 * How long a line stays up before a presenter may move on by itself.
 * 一句台词停留多久后，播放方可以自动进入下一句。
 */
UENUM(BlueprintType)
enum class ESigilDialogueLineDuration : uint8
{
	/** Voice length when the line has a voice, otherwise reading time. 有配音按配音时长，否则按阅读时间。 */
	Default,
	/** Voice length; falls back to reading time when there is no voice. 按配音时长；没有配音时退回阅读时间。 */
	WhenVoiceEnds,
	/** Reading time computed from the text length. 按文字长度算出的阅读时间。 */
	AfterReadingTime,
	/** The seconds typed into DurationSeconds. 使用 DurationSeconds 里填的秒数。 */
	AfterSeconds
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

	/**
	 * Optional voice for this line. The plugin never plays it; presenters load and play it themselves.
	 * 这句台词的配音，可不填。插件自身不播放，由播放方自行加载与播放。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|Presentation")
	TSoftObjectPtr<USoundBase> Voice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|Presentation")
	ESigilDialogueLineDuration LineDuration = ESigilDialogueLineDuration::Default;

	/** Only used when LineDuration is AfterSeconds. 仅当 LineDuration 为 AfterSeconds 时使用。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|Presentation", meta = (ClampMin = "0.0"))
	float DurationSeconds = 0.0f;

	/** Whether a presenter may let the player cut this line short. 播放方是否允许玩家提前跳过这一句。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sigil|Narrative|Presentation")
	bool bSkippable = true;
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

	/**
	 * Seconds a presenter should keep a line up before advancing by itself.
	 * VoiceSeconds is the length of the line's voice as measured by the caller, or 0 when it has none or it is not loaded.
	 * Reading time is text length divided by LettersPerSecond, never below MinReadingSeconds; an empty line reads in 0 seconds.
	 *
	 * 播放方应让一句台词停留多少秒后再自动推进。
	 * VoiceSeconds 是调用方量出的配音时长，没有配音或尚未加载时传 0。
	 * 阅读时间为文字长度除以 LettersPerSecond，且不低于 MinReadingSeconds；空台词的阅读时间为 0。
	 */
	UFUNCTION(BlueprintPure, Category = "Sigil|Narrative")
	static float ResolveLineSeconds(
		const FSigilDialogueNode& Node,
		float VoiceSeconds,
		float LettersPerSecond = 25.0f,
		float MinReadingSeconds = 2.0f);
};
