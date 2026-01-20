// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFHitScanWeapon.h"

#include "Character/OverfrontCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Weapons/Damage/OFWeaponDamageType.h"

#include "DrawDebugHelpers.h"

void AOFHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		if (FireHit.bBlockingHit)
		{
			AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(FireHit.GetActor());
			if (HitCharacter && HasAuthority() && InstigatorController)
			{
				FVector ShotDirection = (HitTarget - Start).GetSafeNormal();
				if (UOFWeaponDamageType* WeaponDamage = DamageType->GetDefaultObject<UOFWeaponDamageType>())
				{
					UGameplayStatics::ApplyPointDamage(HitCharacter, WeaponDamage->BaseDamage, ShotDirection, FireHit, InstigatorController, this, DamageType);
				}
			}
			
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
			}
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

void AOFHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	if (UWorld* World = GetWorld())
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECC_Visibility);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		
		// DrawDebugSphere(GetWorld(), BeamEnd, 6.f, 12, FColor::Orange, true);
		
		if (BeamParticles)
		{
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true))
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}


