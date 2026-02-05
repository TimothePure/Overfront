// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileBullet.h"

#include "Character/OverfrontCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/OFLagCompensationComponent.h"
#include "Particles/ParticleSystem.h"
#include "PlayerController/OFPlayerController.h"
#include "Weapons/Damage/OFWeaponDamageType.h"

AOFProjectileBullet::AOFProjectileBullet() 
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AOFProjectileBullet::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AOFProjectileBullet, InitialSpeed) && ProjectileMovementComponent)
	{
		ProjectileMovementComponent->InitialSpeed = InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = InitialSpeed;
	}
}
#endif

void AOFProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	
	StartLocation = GetActorLocation();
	
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, FName(), GetActorLocation(),
			GetActorRotation(), EAttachLocation::KeepWorldPosition); 
	}

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * ProjectileMovementComponent->InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}

void AOFProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	if (!HasAuthority()) return;
	
	if (AOverfrontCharacter* OwnerCharacter = Cast<AOverfrontCharacter>(GetOwner()))
	{
		if (AOFPlayerController* OwnerController = Cast<AOFPlayerController>(OwnerCharacter->GetController()))
		{
			if (UOFWeaponDamageType* WeaponDamage = DamageType->GetDefaultObject<UOFWeaponDamageType>())
			{
				if (HasAuthority())
				{
					if (bUseServerSideRewind && OwnerController->IsLocalController()) return;
					FVector ShotDirection = (HitResult.ImpactPoint - StartLocation).GetSafeNormal();
					UGameplayStatics::ApplyPointDamage(OtherActor, WeaponDamage->BaseDamage, ShotDirection, HitResult, OwnerController, this, DamageType);
				}
				else if (bUseServerSideRewind)
				{
					if (OwnerCharacter->GetLagCompensationComponent())
					{
						if (AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(OtherActor))
						{
							OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(HitCharacter, StartLocation, HitResult.ImpactPoint, HitResult.BoneName,
								OwnerController->GetServerTime() - OwnerController->SingleTripTime, DamageType);
						}
					}
				}
				// UGameplayStatics::ApplyPointDamage(OtherActor, WeaponDamage->BaseDamage, ShotDirection, HitResult, OwnerController, this, DamageType);
			}
			
		
			// if (OwnerCharacter->HasAuthority())
			// {
			// 	
			// }
			//
			// FVector ShotDirection = (HitResult.ImpactPoint - StartLocation).GetSafeNormal();
			// if (UOFWeaponDamageType* WeaponDamage = DamageType->GetDefaultObject<UOFWeaponDamageType>())
			// {
			// 	UGameplayStatics::ApplyPointDamage(OtherActor, WeaponDamage->BaseDamage, ShotDirection, HitResult, OwnerController, this, DamageType);
			// }
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
}