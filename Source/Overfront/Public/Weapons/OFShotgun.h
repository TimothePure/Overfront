// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFHitScanWeapon.h"
#include "OFShotgun.generated.h"

UCLASS()
class OVERFRONT_API AOFShotgun : public AOFHitScanWeapon
{
	GENERATED_BODY()

public:
	AOFShotgun();
	virtual void FireShotgun(const TArray<FVector_NetQuantize> HitTargets);
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutTargets);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;
};
