// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileRocket.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Weapons/OFRocketMovementComponent.h"


AOFProjectileRocket::AOFProjectileRocket()
{
	PrimaryActorTick.bCanEverTick = false;

	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UOFRocketMovementComponent>(TEXT("Rocket Movement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AOFProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(),
			GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AOFProjectileRocket::OnHit);
	}

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(ProjectileLoop, GetRootComponent(), FName(), GetActorLocation(), EAttachLocation::KeepWorldPosition,
			false, 1.f, 1.f, 0.f, LoopingSoundAttenuation, (USoundConcurrency*)nullptr, false);
	}
}

void AOFProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, Damage / 10,
				GetActorLocation(), DamageInnerRadius, DamageOuterRadius, 1.f, UDamageType::StaticClass(),
				TArray<AActor*>(), this, FiringController);
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AOFProjectileRocket::DestroyTimerFinished, DestroyDuration, false);
	
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent)
	{
		TrailSystemComponent->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

void AOFProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AOFProjectileRocket::Destroyed()
{
	
}