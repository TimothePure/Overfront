// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Damage/OFWeaponDamageType.h"

float UOFWeaponDamageType::DetermineDamageAmount(FName BoneName)
{
	float FinalDamage = BaseDamage;

	if (BoneName.ToString().Contains("head"))
	{
		FinalDamage = HeadDamage;
	}
	else if (BoneName.ToString().Contains("pelvis") || BoneName.ToString().Contains("spine"))
	{
		FinalDamage = TorsoDamage;
	}
	else
	{
		FinalDamage = LimbsDamage;
	}
	
	return FinalDamage;
}
