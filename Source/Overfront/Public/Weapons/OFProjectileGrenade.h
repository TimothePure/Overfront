// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFProjectileExplosive.h"
#include "OFProjectileGrenade.generated.h"

UCLASS()
class OVERFRONT_API AOFProjectileGrenade : public AOFProjectileExplosive
{
	GENERATED_BODY()

public:
	AOFProjectileGrenade();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactNormal);
	
private:
	UPROPERTY(EditAnywhere)
	USoundBase* BounceSound;
};
