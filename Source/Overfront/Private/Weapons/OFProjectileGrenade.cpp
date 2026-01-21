// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileGrenade.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AOFProjectileGrenade::AOFProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AOFProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartExplodeTimer();
	
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AOFProjectileGrenade::OnBounce);
	
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	if (APawn* InstigatorPawn = GetInstigator())
	{
		CollisionBox->IgnoreActorWhenMoving(InstigatorPawn,   true);
	}
	
	GetWorldTimerManager().SetTimer(InstigatorIgnoreCollisionHandle, this, &AOFProjectileGrenade::IgnoreCollisionFinished, 0.3f, false);
}

void AOFProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactNormal)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	}
}

void AOFProjectileGrenade::IgnoreCollisionFinished()
{
	if (APawn* InstigatorPawn = GetInstigator())
	{
		CollisionBox->IgnoreActorWhenMoving(InstigatorPawn, false);
	}
}

