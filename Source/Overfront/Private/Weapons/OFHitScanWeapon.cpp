// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFHitScanWeapon.h"

#include "Character/OverfrontCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"


AOFHitScanWeapon::AOFHitScanWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOFHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOFHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	  
	if (MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;
		FHitResult FireHit;

		if (UWorld* World = GetWorld())
		{
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
			if (FireHit.bBlockingHit)
			{
				if (AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(FireHit.GetActor()))
				{
					if (HasAuthority())
					{
						UGameplayStatics::ApplyDamage(HitCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
					}
				}
				
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}
			}
		}
	}
}

