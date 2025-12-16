// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/OFSpeedPickup.h"

#include "Character/OverfrontCharacter.h"
#include "Components/OFBuffComponent.h"


AOFSpeedPickup::AOFSpeedPickup()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOFSpeedPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (AOverfrontCharacter* Character = Cast<AOverfrontCharacter>(OtherActor))
	{
		if (UOFBuffComponent* BuffComponent = Character->GetBuffComponent())
		{
			BuffComponent->StartSpeedBuff(BaseSpeedBuff, CrouchSpeedBuff, BuffDuration);
		}
	}
	Destroy();
}
