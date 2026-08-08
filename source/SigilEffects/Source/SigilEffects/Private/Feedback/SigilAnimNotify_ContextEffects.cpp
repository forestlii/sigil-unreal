// Copyright (c) 2026 Likeon. All Rights Reserved.


#include "Feedback/SigilAnimNotify_ContextEffects.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Feedback/SigilContextEffectsInterface.h"
#include "Feedback/SigilContextEffectsLibrary.h"
#include "Feedback/SigilContextEffectsPreviewSetting.h"
#include "Feedback/SigilContextEffectsSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SigilAnimNotify_ContextEffects)


bool USigilContextEffectsSpawnParametersProvider::ProvideParameters_Implementation(USkeletalMeshComponent* InMeshComp, const USigilAnimNotify_ContextEffects* InNotifyNotify,
                                                                                  UAnimSequenceBase* InAnimation, FVector& OutSpawnLocation,
                                                                                  FRotator& OutSpawnRotation) const
{
	return false;
}

USigilAnimNotify_ContextEffects::USigilAnimNotify_ContextEffects(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Blue;
#endif
}

void USigilAnimNotify_ContextEffects::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void USigilAnimNotify_ContextEffects::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

FString USigilAnimNotify_ContextEffects::GetNotifyName_Implementation() const
{
	// If the Effect Tag is valid, pass the string name to the notify name
	if (Effect.IsValid())
	{
		return Effect.ToString();
	}

	return Super::GetNotifyName_Implementation();
}

void USigilAnimNotify_ContextEffects::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	bool bValidProvider = !bAttached && IsValid(SpawnParametersProvider) && SpawnParametersProvider->ProvideParameters(MeshComp, this, Animation, SpawnLocation, SpawnRotation);

	if (!bValidProvider && MeshComp->GetOwner())
	{
		SpawnRotation = MeshComp->GetOwner()->GetActorTransform().TransformRotation(RotationOffset.Quaternion()).Rotator();
		SpawnLocation = MeshComp->GetOwner()->GetActorTransform().TransformPosition(LocationOffset);
	}

	// Make sure both MeshComp and Owning Actor is valid
	if (AActor* OwningActor = MeshComp->GetOwner())
	{
		// Prepare Trace Data
		bool bHitSuccess = false;
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;

		if (TraceProperties.bIgnoreActor)
		{
			QueryParams.AddIgnoredActor(OwningActor);
		}

		QueryParams.bReturnPhysicalMaterial = true;

		if (bPerformTrace)
		{
			// If trace is needed, set up Start Location to Attached
			FVector TraceStart = bAttached ? MeshComp->GetSocketLocation(SocketName) + LocationOffset : SpawnLocation;

			// Make sure World is valid
			if (UWorld* World = OwningActor->GetWorld())
			{
				// Call Line Trace, Pass in relevant properties
				bHitSuccess = World->LineTraceSingleByChannel(HitResult, TraceStart, (TraceStart + TraceProperties.EndTraceLocationOffset),
				                                              TraceProperties.TraceChannel, QueryParams, FCollisionResponseParams::DefaultResponseParam);
			}
		}

		// Prepare Contexts in advance
		FGameplayTagContainer SourceContext;

		FGameplayTagContainer TargetContext;

		// Set up Array of Objects that implement the Context Effects Interface
		TArray<UObject*> Implementers;

		// Determine if the Owning Actor is one of the Objects that implements the Context Effects Interface
		if (OwningActor->Implements<USigilContextEffectsInterface>())
		{
			// If so, add it to the Array
			Implementers.Add(OwningActor);
		}

		// Cycle through Owning Actor's Components and determine if any of them is a Component implementing the Context Effect Interface
		for (const auto Component : OwningActor->GetComponents())
		{
			if (Component)
			{
				// If the Component implements the Context Effects Interface, add it to the list
				if (Component->Implements<USigilContextEffectsInterface>())
				{
					Implementers.Add(Component);
				}
			}
		}

		FSigilSpawnContextEffectsInput Input;
		Input.EffectName = Effect;
		Input.bAttached = bAttached;
		Input.Bone = SocketName;
		Input.ComponentToAttach = MeshComp;
		Input.Location = bHitSuccess?HitResult.Location:SpawnLocation;
		Input.Rotation = SpawnRotation;
		Input.LocationOffset = LocationOffset;
		Input.RotationOffset = RotationOffset;
		Input.AnimationSequence = Animation;
		Input.bHitSuccess = bHitSuccess;
		Input.HitResult = HitResult;
		Input.SourceContext = SourceContext;
		Input.TargetContext = TargetContext;
		Input.VFXScale = VFXProperties.Scale;
		Input.AudioVolume = AudioProperties.VolumeMultiplier;
		Input.AudioPitch = AudioProperties.PitchMultiplier;

		// Cycle through all objects implementing the Context Effect Interface
		for (UObject* Implementer : Implementers)
		{
			// If the object is still valid, Execute the AnimMotionEffect Event on it, passing in relevant data
			if (Implementer)
			{
				ISigilContextEffectsInterface::Execute_PlayContextEffectsWithInput(Implementer, Input);
			}
		}

#if WITH_EDITORONLY_DATA
		PerformEditorPreview(OwningActor, SourceContext, TargetContext, MeshComp);
#endif
	}
}

#if WITH_EDITOR
void USigilAnimNotify_ContextEffects::ValidateAssociatedAssets()
{
	Super::ValidateAssociatedAssets();
}

void USigilAnimNotify_ContextEffects::SetParameters(FGameplayTag EffectIn, FVector LocationOffsetIn, FRotator RotationOffsetIn,
                                                   FSigilContextEffectAnimNotifyVFXSettings VFXPropertiesIn, FSigilContextEffectAnimNotifyAudioSettings AudioPropertiesIn,
                                                   bool bAttachedIn, FName SocketNameIn, bool bPerformTraceIn, FSigilContextEffectAnimNotifyTraceSettings TracePropertiesIn)
{
	Effect = EffectIn;
	LocationOffset = LocationOffsetIn;
	RotationOffset = RotationOffsetIn;
	VFXProperties.Scale = VFXPropertiesIn.Scale;
	AudioProperties.PitchMultiplier = AudioPropertiesIn.PitchMultiplier;
	AudioProperties.VolumeMultiplier = AudioPropertiesIn.VolumeMultiplier;
	bAttached = bAttachedIn;
	SocketName = SocketNameIn;
	bPerformTrace = bPerformTraceIn;
	TraceProperties.EndTraceLocationOffset = TracePropertiesIn.EndTraceLocationOffset;
	TraceProperties.TraceChannel = TracePropertiesIn.TraceChannel;
	TraceProperties.bIgnoreActor = TracePropertiesIn.bIgnoreActor;
}

void USigilAnimNotify_ContextEffects::PerformEditorPreview(AActor* OwningActor, FGameplayTagContainer& SourceContext, FGameplayTagContainer& TargetContext, USkeletalMeshComponent* MeshComp)
{
	if (!bAttached)
	{
		return;
	}
	const USigilContextEffectsSettings* ContextEffectsSettings = GetDefault<USigilContextEffectsSettings>();
	if (ContextEffectsSettings == nullptr)
	{
		return;
	}

	// This is for Anim Editor previewing, it is a deconstruction of the calls made by the Interface and the Subsystem
	if (!ContextEffectsSettings->bPreviewInEditor || ContextEffectsSettings->PreviewSetting.IsNull())
		return;

	UWorld* World = OwningActor->GetWorld();

	// Get the world, make sure it's an Editor Preview World
	if (!World || World->WorldType != EWorldType::EditorPreview)
		return;

	// const auto& PreviewProperties = ContextEffectsSettings->PreviewProperties;
	const USigilContextEffectsPreviewSetting* PreviewSetting = ContextEffectsSettings->PreviewSetting.LoadSynchronous();

	if (PreviewSetting == nullptr)
	{
		return;
	}

	// Add Preview contexts if necessary
	SourceContext.AppendTags(PreviewSetting->PreviewSourceContext);
	TargetContext.AppendTags(PreviewSetting->PreviewTargetContext);

	// Convert given Surface Type to Context and Add it to the Contexts for this Preview
	if (PreviewSetting->bPreviewPhysicalSurfaceAsContext)
	{
		TEnumAsByte<EPhysicalSurface> PhysicalSurfaceType = PreviewSetting->PreviewPhysicalSurface;

		if (const FGameplayTag* SurfaceContextPtr = ContextEffectsSettings->SurfaceTypeToContextMap.Find(PhysicalSurfaceType))
		{
			FGameplayTag SurfaceContext = *SurfaceContextPtr;

			SourceContext.AddTag(SurfaceContext);
		}
	}

	for (int i = 0; i < PreviewSetting->PreviewContextEffectsLibraries.Num(); ++i)
	{
		// Libraries are soft referenced, so you will want to try to load them now
		if (UObject* EffectsLibrariesObj = PreviewSetting->PreviewContextEffectsLibraries[i].TryLoad())
		{
			// Check if it is in fact a USigilContextEffectLibrary type
			if (USigilContextEffectsLibrary* EffectLibrary = Cast<USigilContextEffectsLibrary>(EffectsLibrariesObj))
			{
				// Prepare Sounds and Niagara System Arrays
				TArray<USoundBase*> TotalSounds;
				TArray<UNiagaraSystem*> TotalNiagaraSystems;
				TArray<UParticleSystem*> TotalParticleSystems;


				// Attempt to load the Effect Library content (will cache in Transient data on the Effect Library Asset)
				EffectLibrary->LoadEffects();

				// If the Effect Library is valid and marked as Loaded, Get Effects from it
				if (EffectLibrary && EffectLibrary->GetContextEffectsLibraryLoadState() == ESigilContextEffectsLibraryLoadState::Loaded)
				{
					// Prepare local arrays
					TArray<USoundBase*> Sounds;
					TArray<UNiagaraSystem*> NiagaraSystems;
					TArray<UParticleSystem*> ParticleSystems;


					// Get the Effects
					EffectLibrary->GetEffects(Effect, SourceContext, TargetContext, Sounds, NiagaraSystems, ParticleSystems);

					// Append to the accumulating arrays
					TotalSounds.Append(Sounds);
					TotalNiagaraSystems.Append(NiagaraSystems);
					TotalParticleSystems.Append(ParticleSystems);
				}

				// Cycle through Sounds and call Spawn Sound Attached, passing in relevant data
				for (USoundBase* Sound : TotalSounds)
				{
					UGameplayStatics::SpawnSoundAttached(Sound, MeshComp, SocketName, LocationOffset, RotationOffset, EAttachLocation::KeepRelativeOffset,
					                                     false, AudioProperties.VolumeMultiplier, AudioProperties.PitchMultiplier, 0.0f, nullptr, nullptr, true);
				}

				// Cycle through Niagara Systems and call Spawn System Attached, passing in relevant data
				for (UNiagaraSystem* NiagaraSystem : TotalNiagaraSystems)
				{
					UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, MeshComp, SocketName, LocationOffset,
					                                             RotationOffset, VFXProperties.Scale, EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true, true);
				}

				// Cycle through Particle Systems and call Spawn System Attached, passing in relevant data
				for (UParticleSystem* ParticleSystem : TotalParticleSystems)
				{
					UGameplayStatics::SpawnEmitterAttached(ParticleSystem, MeshComp, SocketName, LocationOffset,
					                                       RotationOffset, VFXProperties.Scale, EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None, true);
				}
			}
		}
	}
}
#endif
