// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/OFPickupSpawnPoint.h"

#include "Pickups/OFPickup.h"

AOFPickupSpawnPoint::AOFPickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AOFPickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer();
}

void AOFPickupSpawnPoint::SpawnPickup()
{
	if (PickupClasses.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, PickupClasses.Num() - 1);
		TSubclassOf<AOFPickup> RandomPickupClass = PickupClasses[RandomIndex];
		
		SpawnedPickup = GetWorld()->SpawnActor<AOFPickup>(RandomPickupClass, GetActorTransform());
		if (HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &AOFPickupSpawnPoint::OnPickupDestroyed); 
		}
		
	}
}

void AOFPickupSpawnPoint::StartSpawnPickupTimer()
{
	const float SpawnWaitingDuration = FMath::FRandRange(MinSpawnDuration, MaxSpawnDuration);
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AOFPickupSpawnPoint::SpawnPickupTimerFinished, SpawnWaitingDuration);
}

void AOFPickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

void AOFPickupSpawnPoint::OnPickupDestroyed(AActor* DestroyedActor)
{
	SpawnedPickup = nullptr;
	StartSpawnPickupTimer();
}


