// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFProjectile.h"
#include "OFProjectileExplosive.h"
#include "OFProjectileRocket.generated.h"

UCLASS()
class OVERFRONT_API AOFProjectileRocket : public AOFProjectileExplosive
{
	GENERATED_BODY()

public:
	AOFProjectileRocket();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
	virtual void BeginPlay() override;
	virtual void Explode() override;

	UPROPERTY(EditAnywhere, Category = "Projectile|Properties")
	USoundBase* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere, Category = "Projectile|Properties")
	USoundAttenuation* LoopingSoundAttenuation;
};
