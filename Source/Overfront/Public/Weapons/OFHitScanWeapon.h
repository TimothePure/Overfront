// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFWeapon.h"
#include "OFHitScanWeapon.generated.h"

UCLASS()
class OVERFRONT_API AOFHitScanWeapon : public AOFWeapon
{
	GENERATED_BODY()

public:
	AOFHitScanWeapon();
	virtual void Fire(const FVector& HitTarget) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	
};
