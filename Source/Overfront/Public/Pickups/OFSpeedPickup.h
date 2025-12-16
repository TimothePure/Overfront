// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFPickup.h"
#include "OFSpeedPickup.generated.h"

UCLASS()
class OVERFRONT_API AOFSpeedPickup : public AOFPickup
{
	GENERATED_BODY()

public:
	AOFSpeedPickup();
	
protected:
	virtual void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;
	
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 800.f;
	
	UPROPERTY(EditAnywhere)
	float BuffDuration = 30.f; 
};
