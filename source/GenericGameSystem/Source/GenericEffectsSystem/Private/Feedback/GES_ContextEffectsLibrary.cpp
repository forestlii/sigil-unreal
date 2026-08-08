// Copyright 2025 RedMoonGames All Rights Reserved.


#include "Feedback/GES_ContextEffectsLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GES_ContextEffectsLibrary)


void UGES_ContextEffectsLibrary::GetEffects(const FGameplayTag Effect, const FGameplayTagContainer& SourceContext, const FGameplayTagContainer& TargetContext,
                                            TArray<USoundBase*>& Sounds, TArray<UNiagaraSystem*>& NiagaraSystems, TArray<UParticleSystem*>& ParticleSystems)
{
	// Make sure Effect is valid and Library is loaded
	if (Effect.IsValid() && SourceContext.IsValid() && EffectsLoadState == EGES_ContextEffectsLibraryLoadState::Loaded)
	{
		// Loop through Context Effects
		for (const auto& ActiveContextEffect : ActiveContextEffects)
		{
			bool bMatchesEffectTag = Effect.MatchesTagExact(ActiveContextEffect->EffectTag);
			bool bMatchesSourceContext = ActiveContextEffect->SourceTagQuery.Matches(SourceContext) && (ActiveContextEffect->SourceTagQuery.IsEmpty() == SourceContext.IsEmpty());

			// Target context is optional
			bool bMatchesTargetContext = ActiveContextEffect->TargetTagQuery.IsEmpty() || ActiveContextEffect->TargetTagQuery.Matches(TargetContext);

			// Make sure the Effect is an exact Tag Match and ensure the Context has all tags in the Effect (and neither or both are empty)
			if (bMatchesEffectTag && bMatchesSourceContext && bMatchesTargetContext)
			{
				// Get all Matching Sounds and Niagara Systems
				Sounds.Append(ActiveContextEffect->Sounds);
				NiagaraSystems.Append(ActiveContextEffect->NiagaraSystems);
				ParticleSystems.Append(ActiveContextEffect->ParticleSystems);
			}
		}
	}
}

void UGES_ContextEffectsLibrary::LoadEffects()
{
	// Load Effects into Library if not currently loading
	if (EffectsLoadState != EGES_ContextEffectsLibraryLoadState::Loading)
	{
		// Set load state to loading
		EffectsLoadState = EGES_ContextEffectsLibraryLoadState::Loading;

		// Clear out any old Active Effects
		ActiveContextEffects.Empty();

		// Call internal loading function
		LoadEffectsInternal();
	}
}

EGES_ContextEffectsLibraryLoadState UGES_ContextEffectsLibrary::GetContextEffectsLibraryLoadState()
{
	// Return current Load State
	return EffectsLoadState;
}

void UGES_ContextEffectsLibrary::LoadEffectsInternal()
{
	// TODO Add Async Loading for Libraries

	// Copy data for async load
	TArray<FGES_ContextEffects> LocalContextEffects = ContextEffects;

	// Prepare Active Context Effects Array
	TArray<UGES_ActiveContextEffects*> ActiveContextEffectsArray;

	// Loop through Context Effects
	for (const FGES_ContextEffects& ContextEffect : LocalContextEffects)
	{
		// Make sure Tags are Valid
		if (ContextEffect.EffectTag.IsValid() && !ContextEffect.SourceTagQuery.IsEmpty())
		{
			// Create new Active Context Effect
			UGES_ActiveContextEffects* NewActiveContextEffects = NewObject<UGES_ActiveContextEffects>(this);

			// Pass relevant tag data
			NewActiveContextEffects->EffectTag = ContextEffect.EffectTag;
			NewActiveContextEffects->SourceTagQuery = ContextEffect.SourceTagQuery;
			NewActiveContextEffects->TargetTagQuery = ContextEffect.TargetTagQuery;


			// Try to load and add Effects to New Active Context Effects
			for (const FSoftObjectPath& Effect : ContextEffect.Effects)
			{
				if (UObject* Object = Effect.TryLoad())
				{
					if (Object->IsA(USoundBase::StaticClass()))
					{
						if (USoundBase* SoundBase = Cast<USoundBase>(Object))
						{
							NewActiveContextEffects->Sounds.Add(SoundBase);
						}
					}
					else if (Object->IsA(UNiagaraSystem::StaticClass()))
					{
						if (UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(Object))
						{
							NewActiveContextEffects->NiagaraSystems.Add(NiagaraSystem);
						}
					}
					else if (Object->IsA(UParticleSystem::StaticClass()))
					{
						if (UParticleSystem* ParticleSystem = Cast<UParticleSystem>(Object))
						{
							NewActiveContextEffects->ParticleSystems.Add(ParticleSystem);
						}
					}
				}
			}

			// Add New Active Context to the Active Context Effects Array
			ActiveContextEffectsArray.Add(NewActiveContextEffects);
		}
	}

	// TODO Call Load Complete after Async Load
	// Mark loading complete
	this->OnContextEffectLibraryLoadingComplete(ActiveContextEffectsArray);
}

void UGES_ContextEffectsLibrary::OnContextEffectLibraryLoadingComplete(
	TArray<UGES_ActiveContextEffects*> InActiveContextEffects)
{
	// Flag data as loaded
	EffectsLoadState = EGES_ContextEffectsLibraryLoadState::Loaded;

	// Append incoming Context Effects Array to current list of Active Context Effects
	ActiveContextEffects.Append(InActiveContextEffects);
}

#if WITH_EDITOR
void UGES_ContextEffectsLibrary::PreSave(FObjectPreSaveContext SaveContext)
{
	for (FGES_ContextEffects& ContextEffect : ContextEffects)
	{
		if (!ContextEffect.Context.IsEmpty())
		{
			FGameplayTagQueryExpression Expression;
			Expression.AllTagsMatch().AddTags(ContextEffect.Context);
			ContextEffect.SourceTagQuery.Build(Expression, FString::Format(TEXT("Has all tags({0})"), {ContextEffect.Context.ToStringSimple()}));
			ContextEffect.Context = FGameplayTagContainer();
		}
		ContextEffect.EditorFriendlyName = ContextEffect.EffectTag.IsValid()
			                                   ? FString::Format(TEXT("Effect({0}) Source({1}) Target({2})"),
			                                                     {
				                                                     ContextEffect.EffectTag.GetTagName().ToString(), ContextEffect.SourceTagQuery.GetDescription(),
				                                                     ContextEffect.TargetTagQuery.GetDescription()
			                                                     })
			                                   : TEXT("Invalid Effect");
	}
	Super::PreSave(SaveContext);
}
#endif
