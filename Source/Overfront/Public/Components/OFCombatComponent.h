// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OFCombatComponent.generated.h"

class AOFWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OVERFRONT_API UOFCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOFCombatComponent();
	friend class AOverfrontCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AOFWeapon* WeaponToEquip);
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

private:
	AOverfrontCharacter* Character;

	UPROPERTY(Replicated)
	AOFWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;
};
