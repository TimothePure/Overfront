// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFWeapon.h"
#include "OFProjectileWeapon.generated.h"

UCLASS()
class OVERFRONT_API AOFProjectileWeapon : public AOFWeapon
{
	GENERATED_BODY()

public:
	AOFProjectileWeapon();
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AOFProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AOFProjectile> NotReplicatedProjectileClass;
};
