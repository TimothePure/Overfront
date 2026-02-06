// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/OverfrontCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Overfront/Overfront.h"
#include "Sound/SoundBase.h"
#include "Weapons/ImpactResolver.h"
#include "Weapons/OFWeapon.h"

AOFProjectile::AOFProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);
}

AOFWeapon* AOFProjectile::GetOwningWeapon() const
{
	return Cast<AOFWeapon>(GetOwner());
}

void AOFProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AOFProjectile::OnHit);
	}
}

void AOFProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	ProjectileHitResult = HitResult;
	
	Destroy();
}

void AOFProjectile::Destroyed()
{
	AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(ProjectileHitResult.GetActor());
	FImpactContext Context { ProjectileHitResult, (HitCharacter != nullptr) };
	UImpactResolver::ResolveImpactFX(GetWorld(), Context, ImpactData);
	
	Super::Destroyed();
}

void AOFProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AOFProjectile, DamageType);
}

