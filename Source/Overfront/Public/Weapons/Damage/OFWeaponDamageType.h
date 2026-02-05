// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "OFWeaponDamageType.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFWeaponDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float BaseDamage = 20.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float HeadDamage = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float TorsoDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float LimbsDamage = 15.f;
	
	float DetermineDamageAmount(FName BoneName);
};
