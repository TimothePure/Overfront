// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ImpactResolver.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Overfront/Data/DA_ProjectileImpactsFX.h"

void UImpactResolver::ResolveImpactFX(UWorld* World, const FImpactContext& Context, UDA_ProjectileImpactsFX* ImpactData)
{
	if (!World || !ImpactData) return;

	if (Context.bHitCharacter)
	{
		if (ImpactData->CharacterImpactFX)
		{
			if (UParticleSystem* PS = Cast<UParticleSystem>(ImpactData->CharacterImpactFX))
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, PS, Context.Hit.ImpactPoint, Context.Hit.ImpactNormal.Rotation());
			}
			else if (UNiagaraSystem* NS = Cast<UNiagaraSystem>(ImpactData->CharacterImpactFX))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NS, Context.Hit.ImpactPoint, Context.Hit.ImpactNormal.Rotation());
			}
		}

		if (ImpactData->CharacterImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(World, ImpactData->CharacterImpactSound,Context.Hit.ImpactPoint);
		}
		return;
	}

	const EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Context.Hit.PhysMaterial.Get());

	for (const FProjectileImpactData& Data : ImpactData->SurfaceImpacts)
	{
		if (Data.SurfaceType == SurfaceType)
		{
			if (Data.ImpactFX)
			{
				if (UParticleSystem* PS = Cast<UParticleSystem>(Data.ImpactFX))
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, PS, Context.Hit.ImpactPoint, Context.Hit.ImpactNormal.Rotation());
				}
				else if (UNiagaraSystem* NS = Cast<UNiagaraSystem>(Data.ImpactFX))
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NS, Context.Hit.ImpactPoint, Context.Hit.ImpactNormal.Rotation());
				}
			}  

			if (Data.ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(World, Data.ImpactSound, Context.Hit.ImpactPoint);
			}
			break;
		}
	}
}
