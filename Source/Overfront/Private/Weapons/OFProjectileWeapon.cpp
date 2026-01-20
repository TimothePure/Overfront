// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/OFProjectile.h"
#include "Weapons/OFProjectileBullet.h"


AOFProjectileWeapon::  AOFProjectileWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AOFProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzfleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzfleFlashSocket)
	{
		FTransform SocketTransform = MuzzfleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From Muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		
		if (ProjectileClass && InstigatorPawn)
		{
			if (UWorld* World = GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = InstigatorPawn;
				if (AOFProjectile* SpawnedProjectile = World->SpawnActor<AOFProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams))
				{
					SpawnedProjectile->DamageType = DamageType;
				}
			}
		}
	}
}

void AOFProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOFProjectileWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

