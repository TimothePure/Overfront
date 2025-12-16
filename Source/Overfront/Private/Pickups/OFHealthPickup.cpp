// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/OFHealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/OverfrontCharacter.h"
#include "Components/OFBuffComponent.h"


AOFHealthPickup::AOFHealthPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	PickupEffectComp = CreateDefaultSubobject<UNiagaraComponent>("PickupEffectComp");
	PickupEffectComp->SetupAttachment(RootComponent);
}

void AOFHealthPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOFHealthPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (AOverfrontCharacter* Character = Cast<AOverfrontCharacter>(OtherActor))
	{
		if (UOFBuffComponent* BuffComponent = Character->GetBuffComponent())
		{
			BuffComponent->Heal(100.f, 3.f);
		}
	}
	Destroy();
}

void AOFHealthPickup::Destroyed()
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());
	}
	Super::Destroyed();
}