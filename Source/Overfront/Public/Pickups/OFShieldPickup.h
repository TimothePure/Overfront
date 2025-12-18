// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFPickup.h"
#include "OFShieldPickup.generated.h"

UCLASS()
class OVERFRONT_API AOFShieldPickup : public AOFPickup
{
	GENERATED_BODY()

public:
	AOFShieldPickup();

protected:
	virtual void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float ShieldAmount = 100.f;
	
	UPROPERTY(EditAnywhere)
	float ShieldDuration = 5.f;
};
