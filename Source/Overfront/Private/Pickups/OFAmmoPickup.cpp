// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/OFAmmoPickup.h"

#include "Character/OverfrontCharacter.h"
#include "Components/OFCombatComponent.h"


AOFAmmoPickup::AOFAmmoPickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AOFAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
}

void AOFAmmoPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	if (AOverfrontCharacter* Character = Cast<AOverfrontCharacter>(OtherActor))
	{
		if (UOFCombatComponent* CombatComponent = Character->GetCombatComponent())
		{
			CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}

