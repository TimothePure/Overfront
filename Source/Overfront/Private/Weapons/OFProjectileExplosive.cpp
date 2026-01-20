// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileExplosive.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Damage/OFWeaponDamageType.h"

void AOFProjectileExplosive::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnTrailSystem();
}

void AOFProjectileExplosive::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(),
			GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
}

void AOFProjectileExplosive::StartExplodeTimer()
{
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AOFProjectileExplosive::ExplodeTimerFinished, ExplodeDuration, false);
}

void AOFProjectileExplosive::ExplodeTimerFinished()
{
	Explode();
}

void AOFProjectileExplosive::Explode()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, BaseDamage,  BaseDamage / 4,
			GetActorLocation(), DamageInnerRadius, DamageOuterRadius, 1.f, UDamageType::StaticClass(),
			TArray<AActor*>(), this, FiringController);
		}
	}
	
	StartDestroyTimer();

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent)
	{
		TrailSystemComponent->Deactivate();
	}
}

void AOFProjectileExplosive::StartDestroyTimer()
{
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AOFProjectileExplosive::DestroyTimerFinished, DestroyDuration, false);
}

void AOFProjectileExplosive::DestroyTimerFinished()
{
	Destroy();
}

void AOFProjectileExplosive::Destroyed()
{
	// Do nothing (projectile base class spawns hit sound and particles in here)
}