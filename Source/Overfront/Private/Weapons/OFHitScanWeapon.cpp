// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFHitScanWeapon.h"

#include "Character/OverfrontCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Weapons/Damage/OFWeaponDamageType.h"

#include "DrawDebugHelpers.h"
#include "Components/OFLagCompensationComponent.h"
#include "PlayerController/OFPlayerController.h"
#include "Weapons/ImpactResolver.h"

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
			if (HitCharacter && InstigatorController)
			{
				if (UOFWeaponDamageType* WeaponDamage = DamageType->GetDefaultObject<UOFWeaponDamageType>())
				{
					if (HasAuthority())
					{
						if (bUseServerSideRewind && OwnerPawn && !OwnerPawn->IsLocallyControlled()) return;

						UGameplayStatics::ApplyDamage(HitCharacter, WeaponDamage->DetermineDamageAmount(FireHit.BoneName), InstigatorController, this, DamageType);
					}
					else if (bUseServerSideRewind)
					{
						OwnerCharacter = OwnerCharacter == nullptr ? Cast<AOverfrontCharacter>(OwnerPawn) : OwnerCharacter;
						OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AOFPlayerController>(InstigatorController): OwnerPlayerController;
						
						if (OwnerCharacter && OwnerPlayerController && OwnerCharacter->GetLagCompensationComponent())
						{
							OwnerCharacter->GetLagCompensationComponent()->HitscanServerScoreRequest(HitCharacter, Start, FireHit.ImpactPoint, FireHit.BoneName,
								OwnerPlayerController->GetServerTime() - OwnerPlayerController->SingleTripTime,this);
						}
					}
				}
			}
			
			FImpactContext Context { FireHit, (HitCharacter != nullptr) };
			UImpactResolver::ResolveImpactFX(GetWorld(), Context, ImpactData);
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
		
		if (BeamParticles)
		{
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true))
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}


