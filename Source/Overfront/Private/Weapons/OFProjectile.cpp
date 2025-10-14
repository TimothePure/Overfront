// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectile.h"

#include "Components/BoxComponent.h"

AOFProjectile::AOFProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

void AOFProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOFProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

