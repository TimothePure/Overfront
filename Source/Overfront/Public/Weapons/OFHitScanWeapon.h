// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFWeapon.h"
#include "Overfront/Data/DA_ProjectileImpactsFX.h"
#include "OFHitScanWeapon.generated.h"


UCLASS()
class OVERFRONT_API AOFHitScanWeapon : public AOFWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere)
	UDA_ProjectileImpactsFX* ImpactData;
	
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;
};
