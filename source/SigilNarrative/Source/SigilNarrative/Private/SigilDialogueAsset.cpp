// Copyright (c) 2026 Likeon. All Rights Reserved.

#include "SigilDialogueAsset.h"

#include "SigilNarrativeCondition.h"
#include "SigilNarrativeEvent.h"

bool USigilDialogueAsset::ValidateDefinition(FText& OutError) const
{
	auto Fail = [&OutError](const FString& Message)
	{
		OutError = FText::FromString(Message);
		return false;
	};

	OutError = FText::GetEmpty();
	if (DialogueId.IsNone())
	{
		return Fail(TEXT("DialogueId must not be empty."));
	}
	if (EntryNodeId.IsNone())
	{
		return Fail(TEXT("EntryNodeId must not be empty."));
	}

	TSet<FName> NodeIds;
	for (const FSigilDialogueNode& Node : Nodes)
	{
		if (Node.NodeId.IsNone())
		{
			return Fail(TEXT("NodeId must not be empty."));
		}
		if (NodeIds.Contains(Node.NodeId))
		{
			return Fail(FString::Printf(TEXT("Duplicate NodeId: %s."), *Node.NodeId.ToString()));
		}
		NodeIds.Add(Node.NodeId);

		switch (Node.NodeType)
		{
		case ESigilDialogueNodeType::Line:
			if (Node.NextNodeId.IsNone())
			{
				return Fail(FString::Printf(TEXT("Line node %s must have a target."), *Node.NodeId.ToString()));
			}
			break;

		case ESigilDialogueNodeType::Choice:
			if (Node.Options.IsEmpty())
			{
				return Fail(FString::Printf(TEXT("Choice node %s must have at least one option."), *Node.NodeId.ToString()));
			}
			break;

		case ESigilDialogueNodeType::End:
			if (!Node.NextNodeId.IsNone() || !Node.Options.IsEmpty())
			{
				return Fail(FString::Printf(TEXT("End node %s cannot have targets or options."), *Node.NodeId.ToString()));
			}
			break;

		default:
			return Fail(FString::Printf(TEXT("Node %s has an unknown node type."), *Node.NodeId.ToString()));
		}

		TSet<FName> OptionIds;
		for (const FSigilDialogueOption& Option : Node.Options)
		{
			if (Option.OptionId.IsNone())
			{
				return Fail(FString::Printf(TEXT("Choice node %s has an empty OptionId."), *Node.NodeId.ToString()));
			}
			if (OptionIds.Contains(Option.OptionId))
			{
				return Fail(FString::Printf(TEXT("Choice node %s has duplicate OptionId %s."), *Node.NodeId.ToString(), *Option.OptionId.ToString()));
			}
			OptionIds.Add(Option.OptionId);
			if (Option.TargetNodeId.IsNone())
			{
				return Fail(FString::Printf(TEXT("Option %s must have a target."), *Option.OptionId.ToString()));
			}
			for (const USigilNarrativeCondition* Condition : Option.Conditions)
			{
				if (!Condition)
				{
					return Fail(FString::Printf(TEXT("Option %s contains a null condition."), *Option.OptionId.ToString()));
				}
			}
			for (const USigilNarrativeEvent* Event : Option.Events)
			{
				if (!Event)
				{
					return Fail(FString::Printf(TEXT("Option %s contains a null event."), *Option.OptionId.ToString()));
				}
			}
		}
	}

	if (!NodeIds.Contains(EntryNodeId))
	{
		return Fail(TEXT("EntryNodeId must reference an existing node."));
	}

	for (const FSigilDialogueNode& Node : Nodes)
	{
		if (Node.NodeType == ESigilDialogueNodeType::Line && !NodeIds.Contains(Node.NextNodeId))
		{
			return Fail(FString::Printf(TEXT("Line node %s references a missing target."), *Node.NodeId.ToString()));
		}
		for (const FSigilDialogueOption& Option : Node.Options)
		{
			if (!NodeIds.Contains(Option.TargetNodeId))
			{
				return Fail(FString::Printf(TEXT("Option %s references a missing target."), *Option.OptionId.ToString()));
			}
		}
	}

	return true;
}

const FSigilDialogueNode* USigilDialogueAsset::FindNode(const FName NodeId) const
{
	return Nodes.FindByPredicate([NodeId](const FSigilDialogueNode& Node)
	{
		return Node.NodeId == NodeId;
	});
}
