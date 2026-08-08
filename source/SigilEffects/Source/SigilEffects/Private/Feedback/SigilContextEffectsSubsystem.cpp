// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Feedback/SigilContextEffectsSubsystem.h"

#include "Feedback/SigilContextEffectsLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilContextEffectsSubsystem)

class AActor;
class UAudioComponent;
class UNiagaraSystem;
class USceneComponent;
class USoundBase;

void USigilContextEffectsSubsystem::SpawnContextEffects(UObject* WorldContextObject, TSoftObjectPtr<USigilContextEffectsLibrary> EffectsLibrary, FSigilSpawnContextEffectsInput Input,
                                                       FSigilSpawnContextEffectsOutput& Output)
{
	if (WorldContextObject == nullptr || WorldContextObject->GetWorld() == nullptr)
	{
		return;
	}
	
	if (EffectsLibrary.IsNull())
	{
		return;
	}

	// Prepare Arrays for Sounds and Niagara Systems
	TArray<USoundBase*> TotalSounds;
	TArray<UNiagaraSystem*> TotalNiagaraSystems;
	TArray<UParticleSystem*> TotalParticleSystems;

	// Cycle through Effect Libraries
	if (USigilContextEffectsLibrary* EffectLibrary = EffectsLibrary.LoadSynchronous())
	{
		if (EffectLibrary && EffectLibrary->GetContextEffectsLibraryLoadState() == ESigilContextEffectsLibraryLoadState::Unloaded)
		{
			// Sync load effects
			EffectLibrary->LoadEffects();
		}

		// Check if the Effect Library is valid and data Loaded
		if (EffectLibrary->GetContextEffectsLibraryLoadState() == ESigilContextEffectsLibraryLoadState::Loaded)
		{
			// Set up local list of Sounds and Niagara Systems
			TArray<USoundBase*> Sounds;
			TArray<UNiagaraSystem*> NiagaraSystems;
			TArray<UParticleSystem*> ParticleSystems;

			// Get Sounds and Niagara Systems
			EffectLibrary->GetEffects(Input.EffectName, Input.SourceContext, Input.TargetContext, Sounds, NiagaraSystems, ParticleSystems);

			// Append to accumulating array
			TotalSounds.Append(Sounds);
			TotalNiagaraSystems.Append(NiagaraSystems);
			TotalParticleSystems.Append(ParticleSystems);
		}
	}

	// Cycle through found Sounds
	for (USoundBase* Sound : TotalSounds)
	{
		if (Input.bAttached)
		{
			// Spawn Sounds Attached, add Audio Component to List of ACs
			UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(Sound, Input.ComponentToAttach, Input.Bone, Input.LocationOffset, Input.RotationOffset,
			                                                                       EAttachLocation::KeepRelativeOffset,
			                                                                       false, Input.AudioVolume, Input.AudioPitch, 0.0f, nullptr, nullptr, true);
			Output.AudioComponents.Add(AudioComponent);
		}
		else
		{
			UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(WorldContextObject, Sound, Input.Location, Input.Rotation, Input.AudioVolume, Input.AudioPitch, 0.0f, nullptr,
			                                                                         nullptr, true);
			Output.AudioComponents.Add(AudioComponent);
		}
	}

	// Cycle through found Niagara Systems
	for (UNiagaraSystem* NiagaraSystem : TotalNiagaraSystems)
	{
		if (Input.bAttached)
		{
			// Spawn Niagara Systems Attached, add Niagara Component to List of NCs
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, Input.ComponentToAttach, Input.Bone, Input.LocationOffset,
			                                                                                   Input.RotationOffset, Input.VFXScale, EAttachLocation::KeepRelativeOffset, true,
			                                                                                   ENCPoolMethod::None,
			                                                                                   true,
			                                                                                   true);
			Output.NiagaraComponents.Add(NiagaraComponent);
		}
		else
		{
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObject, NiagaraSystem, Input.Location,
			                                                                                     Input.Rotation, Input.VFXScale, true, true,
			                                                                                     ENCPoolMethod::None, true);

			Output.NiagaraComponents.Add(NiagaraComponent);
		}
	}

	// Cycle through found Particle Systems
	for (UParticleSystem* ParticleSystem : TotalParticleSystems)
	{
		if (Input.bAttached)
		{
			// Spawn Particle Systems Attached, add Niagara Component to List of NCs
			UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAttached(ParticleSystem, Input.ComponentToAttach, Input.Bone, Input.LocationOffset,
			                                                                                     Input.RotationOffset, Input.VFXScale, EAttachLocation::KeepRelativeOffset, true,
			                                                                                     EPSCPoolMethod::None,
			                                                                                     true);
			Output.ParticlesComponents.Add(ParticleComponent);
		}
		else
		{
			UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(WorldContextObject, ParticleSystem, Input.Location, Input.Rotation, Input.VFXScale, true,
			                                                                                       EPSCPoolMethod::None, true);
			Output.ParticlesComponents.Add(ParticleComponent);
		}
	}
}

void USigilContextEffectsSubsystem::SpawnContextEffectsExt(const AActor* SpawningActor, const FSigilSpawnContextEffectsInput& Input, FSigilSpawnContextEffectsOutput& Output)
{
	// First determine if this Actor has a matching Set of Libraries
	if (TObjectPtr<USigilContextEffectsSet>* EffectsLibrariesSetPtr = ActiveActorEffectsMap.Find(SpawningActor))
	{
		// Validate the pointers from the Map Find
		if (USigilContextEffectsSet* EffectsLibraries = *EffectsLibrariesSetPtr)
		{
			// Prepare Arrays for Sounds and Niagara Systems
			TArray<USoundBase*> TotalSounds;
			TArray<UNiagaraSystem*> TotalNiagaraSystems;
			TArray<UParticleSystem*> TotalParticleSystems;

			// Cycle through Effect Libraries
			for (USigilContextEffectsLibrary* EffectLibrary : EffectsLibraries->ContextEffectsLibraries)
			{
				// Check if the Effect Library is valid and data Loaded
				if (EffectLibrary && EffectLibrary->GetContextEffectsLibraryLoadState() == ESigilContextEffectsLibraryLoadState::Loaded)
				{
					// Set up local list of Sounds and Niagara Systems
					TArray<USoundBase*> Sounds;
					TArray<UNiagaraSystem*> NiagaraSystems;
					TArray<UParticleSystem*> ParticleSystems;

					// Get Sounds and Niagara Systems
					EffectLibrary->GetEffects(Input.EffectName, Input.SourceContext, Input.TargetContext, Sounds, NiagaraSystems, ParticleSystems);

					// Append to accumulating array
					TotalSounds.Append(Sounds);
					TotalNiagaraSystems.Append(NiagaraSystems);
					TotalParticleSystems.Append(ParticleSystems);
				}
				else if (EffectLibrary && EffectLibrary->GetContextEffectsLibraryLoadState() == ESigilContextEffectsLibraryLoadState::Unloaded)
				{
					// Else load effects
					EffectLibrary->LoadEffects();
				}
			}

			// Cycle through found Sounds
			for (USoundBase* Sound : TotalSounds)
			{
				if (Input.bAttached)
				{
					// Spawn Sounds Attached, add Audio Component to List of ACs
					UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(Sound, Input.ComponentToAttach, Input.Bone, Input.LocationOffset, Input.RotationOffset,
					                                                                       EAttachLocation::KeepRelativeOffset,
					                                                                       false, Input.AudioVolume, Input.AudioPitch, 0.0f, nullptr, nullptr, true);
					Output.AudioComponents.Add(AudioComponent);
				}
				else
				{
					UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(SpawningActor, Sound, Input.Location, Input.Rotation, Input.AudioVolume, Input.AudioPitch, 0.0f, nullptr,
					                                                                         nullptr, true);
					Output.AudioComponents.Add(AudioComponent);
				}
			}

			// Cycle through found Niagara Systems
			for (UNiagaraSystem* NiagaraSystem : TotalNiagaraSystems)
			{
				if (Input.bAttached)
				{
					// Spawn Niagara Systems Attached, add Niagara Component to List of NCs
					UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, Input.ComponentToAttach, Input.Bone, Input.LocationOffset,
					                                                                                   Input.RotationOffset, Input.VFXScale, EAttachLocation::KeepRelativeOffset, true,
					                                                                                   ENCPoolMethod::None,
					                                                                                   true,
					                                                                                   true);
					Output.NiagaraComponents.Add(NiagaraComponent);
				}
				else
				{
					UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(SpawningActor, NiagaraSystem, Input.Location,
					                                                                                     Input.Rotation, Input.VFXScale, true, true,
					                                                                                     ENCPoolMethod::None, true);

					Output.NiagaraComponents.Add(NiagaraComponent);
				}
			}

			// Cycle through found Particle Systems
			for (UParticleSystem* ParticleSystem : TotalParticleSystems)
			{
				if (Input.bAttached)
				{
					// Spawn Particle Systems Attached, add Niagara Component to List of NCs
					UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAttached(ParticleSystem, Input.ComponentToAttach, Input.Bone, Input.LocationOffset,
					                                                                                     Input.RotationOffset, Input.VFXScale, EAttachLocation::KeepRelativeOffset, true,
					                                                                                     EPSCPoolMethod::None,
					                                                                                     true);
					Output.ParticlesComponents.Add(ParticleComponent);
				}
				else
				{
					UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(SpawningActor, ParticleSystem, Input.Location, Input.Rotation, Input.VFXScale, true,
					                                                                                       EPSCPoolMethod::None, true);
					Output.ParticlesComponents.Add(ParticleComponent);
				}
			}
		}
	}
}

bool USigilContextEffectsSubsystem::GetContextFromSurfaceType(
	TEnumAsByte<EPhysicalSurface> PhysicalSurface, FGameplayTag& Context)
{
	// Get Project Settings
	if (const USigilContextEffectsSettings* ProjectSettings = GetDefault<USigilContextEffectsSettings>())
	{
		// Find which Gameplay Tag the Surface Type is mapped to
		if (const FGameplayTag* GameplayTagPtr = ProjectSettings->SurfaceTypeToContextMap.Find(PhysicalSurface))
		{
			Context = *GameplayTagPtr;
		}
	}

	// Return true if Context is Valid
	return Context.IsValid();
}

void USigilContextEffectsSubsystem::LoadAndAddContextEffectsLibraries(AActor* OwningActor,
                                                                     TSet<TSoftObjectPtr<USigilContextEffectsLibrary>> ContextEffectsLibraries)
{
	// Early out if Owning Actor is invalid or if the associated Libraries is 0 (or less)
	if (OwningActor == nullptr || ContextEffectsLibraries.Num() <= 0)
	{
		return;
	}

	// Create new Context Effect Set
	USigilContextEffectsSet* EffectsLibrariesSet = NewObject<USigilContextEffectsSet>(this);

	// Cycle through Libraries getting Soft Obj Refs
	for (const TSoftObjectPtr<USigilContextEffectsLibrary>& ContextEffectSoftObj : ContextEffectsLibraries)
	{
		// Load Library Assets from Soft Obj refs
		// TODO Support Async Loading of Asset Data
		if (USigilContextEffectsLibrary* EffectsLibrary = ContextEffectSoftObj.LoadSynchronous())
		{
			// Call load on valid Libraries
			EffectsLibrary->LoadEffects();

			// Add new library to Set
			EffectsLibrariesSet->ContextEffectsLibraries.Add(EffectsLibrary);
		}
	}

	// Update Active Actor Effects Map
	ActiveActorEffectsMap.Emplace(OwningActor, EffectsLibrariesSet);
}

void USigilContextEffectsSubsystem::UnloadAndRemoveContextEffectsLibraries(AActor* OwningActor)
{
	// Early out if Owning Actor is invalid
	if (OwningActor == nullptr)
	{
		return;
	}

	// Remove ref from Active Actor/Effects Set Map
	ActiveActorEffectsMap.Remove(OwningActor);
}
