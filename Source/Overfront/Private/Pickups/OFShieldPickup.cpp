// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/OFShieldPickup.h"

#include "Character/OverfrontCharacter.h"
#include "Components/OFBuffComponent.h"


AOFShieldPickup::AOFShieldPickup()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOFShieldPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (AOverfrontCharacter* Character = Cast<AOverfrontCharacter>(OtherActor))
	{
		if (UOFBuffComponent* BuffComponent = Character->GetBuffComponent())
		{
			BuffComponent->ReplenishShield(ShieldAmount, ShieldDuration);
		}
	}
	Destroy();
}

