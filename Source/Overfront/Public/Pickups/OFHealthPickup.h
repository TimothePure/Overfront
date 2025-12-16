// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFPickup.h"
#include "OFHealthPickup.generated.h"

UCLASS()
class OVERFRONT_API AOFHealthPickup : public AOFPickup
{
	GENERATED_BODY()

public:
	AOFHealthPickup();
	
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;
	
	UPROPERTY(EditAnywhere)
	float HealingDuration = 5.f;
	
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComp;
	
	UPROPERTY(EditAnywhere )
	class UNiagaraSystem* PickupEffect;
};
