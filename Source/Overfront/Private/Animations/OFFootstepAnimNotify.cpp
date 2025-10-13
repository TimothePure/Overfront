// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/OFFootstepAnimNotify.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void UOFFootstepAnimNotify::PlaySoundFromSurfaceType(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float Volume) 
{
	if (!MeshComp) return;
	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	const FVector Start = MeshComp->GetComponentLocation();
	const FVector End = Start - FVector(0.f, 0.f, TraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepTrace), false, MeshComp->GetOwner());
	Params.bReturnPhysicalMaterial = true;

	bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

#if WITH_EDITOR
	DrawDebugLine(World, Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);
#endif

	if (!bHit) return;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
	USoundBase* SoundToPlay = DefaultSound;

	switch (SurfaceType)
	{
	case SurfaceType1 :
		SoundToPlay = DirtSound;
		break;
	case SurfaceType2:
		SoundToPlay = WoodSound;
		break;
	case SurfaceType3:
		SoundToPlay = SlushSound;
		break;
	default:
		SoundToPlay = DefaultSound;
		break;
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SoundToPlay, Hit.ImpactPoint, Volume);
	}
}
