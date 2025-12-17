// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFPickup.h"
#include "UObject/Object.h"
#include "OFJumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API AOFJumpPickup : public AOFPickup
{
	GENERATED_BODY()
	
public:
	AOFJumpPickup();
	
protected:
	virtual void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere)
	float JumpZVelocity = 1000.f;
	
	UPROPERTY(EditAnywhere)
	float BuffDuration = 30.f;
};
