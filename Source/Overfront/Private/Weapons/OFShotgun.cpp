// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFShotgun.h"

#include "Character/OverfrontCharacter.h"
#include "Components/OFLagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerController/OFPlayerController.h"
#include "Weapons/ImpactResolver.h"
#include "Weapons/Damage/OFWeaponDamageType.h"


AOFShotgun::AOFShotgun()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOFShotgun::BeginPlay()
{
	Super::BeginPlay();
}

void AOFShotgun::FireShotgun(const TArray<FVector_NetQuantize> HitTargets)
{
	AOFWeapon::Fire(FVector());
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		for (auto HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			if (FireHit.bBlockingHit)
			{
				FVector ShotDirection = (HitTarget - Start).GetSafeNormal();
				AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(FireHit.GetActor());
				if (HitCharacter && InstigatorController)
				{
					if (UOFWeaponDamageType* WeaponDamage = DamageType->GetDefaultObject<UOFWeaponDamageType>())
					{
						if (HasAuthority())
						{
							if (bUseServerSideRewind && OwnerPawn && !OwnerPawn->IsLocallyControlled()) return;
						
							UGameplayStatics::ApplyPointDamage(HitCharacter, WeaponDamage->BaseDamage, ShotDirection, FireHit, InstigatorController, this, DamageType);
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
		}
	}
}

void AOFShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		const FVector ToEndLoc = EndLoc - TraceStart;
		const FVector Target = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
		
		OutTargets.Add(Target);
	}
}
