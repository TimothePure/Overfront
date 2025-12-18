// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OFPickupSpawnPoint.generated.h"


UCLASS()
class OVERFRONT_API AOFPickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AOFPickupSpawnPoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AOFPickup>> PickupClasses;
	
	void SpawnPickup();
private:
	FTimerHandle SpawnTimerHandle;
	void StartSpawnPickupTimer();
	void SpawnPickupTimerFinished();
	
	UPROPERTY(EditAnywhere)
	float MinSpawnDuration = 10.f;
	
	UPROPERTY(EditAnywhere)
	float MaxSpawnDuration = 30.f;
	
	UPROPERTY()
	AOFPickup* SpawnedPickup;
	
	UFUNCTION()
	void OnPickupDestroyed(AActor* DestroyedActor);
};
