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
#include "Weapons/OFWeapon.h"
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
}

void AOFProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	AOverfrontCharacter* OwnerCharacter = Cast<AOverfrontCharacter>(GetInstigator());
	AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(OtherActor);
	
	if (HitCharacter && OwnerCharacter)
	{
		if (AOFPlayerController* OwnerController = Cast<AOFPlayerController>(OwnerCharacter->GetController()))
		{
			if (UOFWeaponDamageType* WeaponDamage = DamageType->GetDefaultObject<UOFWeaponDamageType>())
			{
				if (HasAuthority())
				{
					if (!bUseServerSideRewind || !OwnerController->IsLocalController())
					{
						AOFWeapon* WeaponInstigator = Cast<AOFWeapon>(GetOwner());
						UGameplayStatics::ApplyDamage(HitCharacter, WeaponDamage->DetermineDamageAmount(HitResult.BoneName), OwnerController, WeaponInstigator, WeaponInstigator->DamageType);
					}
				}
				else if (bUseServerSideRewind && OwnerCharacter->GetLagCompensationComponent())
				{
					OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(HitCharacter, StartLocation, HitResult.ImpactPoint, HitResult.BoneName,
						OwnerController->GetServerTime() - OwnerController->SingleTripTime, GetOwningWeapon());
				}
			}
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
}