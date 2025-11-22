// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AOFProjectileBullet::AOFProjectileBullet()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AOFProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (AController* OwnerController = OwnerCharacter->GetController())
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
}


