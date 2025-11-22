// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFProjectile.h"
#include "OFProjectileBullet.generated.h"

UCLASS()
class OVERFRONT_API AOFProjectileBullet : public AOFProjectile
{
	GENERATED_BODY()

public:
	AOFProjectileBullet();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
};
