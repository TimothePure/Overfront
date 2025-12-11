// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFPickup.h"
#include "Overfront/Enums/OFWeaponTypes.h"
#include "OFAmmoPickup.generated.h"

UCLASS()
class OVERFRONT_API AOFAmmoPickup : public AOFPickup
{
	GENERATED_BODY()

public:
	AOFAmmoPickup();

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};
