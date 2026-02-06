// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/OFProjectile.h"
#include "Weapons/OFProjectileBullet.h"


AOFProjectileWeapon::AOFProjectileWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AOFProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzfleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	
	if (MuzzfleFlashSocket == nullptr || World == nullptr || InstigatorPawn == nullptr) return;
	if (ProjectileClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Missing Projectile Class"));
	}
	
	if (NotReplicatedProjectileClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Missing NotReplicated Projectile Class"));
	}
	 
	FTransform SocketTransform = MuzzfleFlashSocket->GetSocketTransform(GetWeaponMesh());
	// From Muzzle flash socket to hit location from TraceUnderCrosshairs
	FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	FRotator TargetRotation = ToTarget.Rotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = InstigatorPawn;
	
	AOFProjectile* SpawnedProjectile = nullptr;
	if (bUseServerSideRewind)
	{
		if (InstigatorPawn->HasAuthority()) // server
		{
			if (InstigatorPawn->IsLocallyControlled()) // server host, use replicated projectile
			{
				SpawnedProjectile = World->SpawnActor<AOFProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				if (SpawnedProjectile)
				{
					SpawnedProjectile->DamageType = DamageType;
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			} else // server, not locally controlled, spawn non-replicated projectile, no SSR
			{
				SpawnedProjectile = World->SpawnActor<AOFProjectile>(NotReplicatedProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				if (SpawnedProjectile)
				{
					SpawnedProjectile->DamageType = DamageType;
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		} else // client using SSR
		{
			if (InstigatorPawn->IsLocallyControlled()) // client, locally controlled, spawn non-replicated projectile and use SSR
			{
				SpawnedProjectile = World->SpawnActor<AOFProjectile>(NotReplicatedProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				if (SpawnedProjectile)
				{
					SpawnedProjectile->DamageType = DamageType;
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				}
			} else // client not locally controlled, spawn non-replicated projectile, no SSR
			{
				SpawnedProjectile = World->SpawnActor<AOFProjectile>(NotReplicatedProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				
				if (SpawnedProjectile)
				{
					SpawnedProjectile->DamageType = DamageType;
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
	} else // weapon not using SSR
	{
		if (InstigatorPawn->HasAuthority())
		{
			SpawnedProjectile = World->SpawnActor<AOFProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
			if (SpawnedProjectile)
			{
				SpawnedProjectile->DamageType = DamageType;
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}
	}	
}

