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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AOFProjectile> ProjectileClass;
};
