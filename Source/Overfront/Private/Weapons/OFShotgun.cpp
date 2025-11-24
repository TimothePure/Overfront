// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFShotgun.h"

#include "Character/OverfrontCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


AOFShotgun::AOFShotgun()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOFShotgun::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOFShotgun::Fire(const FVector& HitTarget)
{
	AOFWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	  
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		TMap<AOverfrontCharacter*, uint32> HitMap;

		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			if (FireHit.bBlockingHit)
			{
				AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(FireHit.GetActor());
				if (HitCharacter && HasAuthority() && InstigatorController)
				{
					HitMap.FindOrAdd(HitCharacter)++;
				}

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
				}
			}
		}

		if (HasAuthority() && InstigatorController)
		{
			for (auto HitPair : HitMap)
			{
				if (HitPair.Key)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
				}
			}
		}
		
		
	}
}
