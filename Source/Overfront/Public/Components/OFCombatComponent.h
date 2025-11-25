// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Overfront/Enums/OFCombatStates.h"
#include "Overfront/Enums/OFWeaponTypes.h"
#include "Widgets/OFHUD.h"
#include "OFCombatComponent.generated.h"


#define CAMERA_IGNORE_DISTANCE = 350.f;

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
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	

	void FireInput(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& HitTarget);

	void TraceUnderCrosshairs(FHitResult& HitResult);

	void SetHUDCrosshair(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 AmountToReload();
private:
	class AOverfrontCharacter* Character;
	class AOFPlayerController* Controller;
	class AOFHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AOFWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireInputPressed;

	// HUD and Crosshair
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairShootingFactor;
	FHUDPackage HUDPackage;

	FVector Target;
	// Aiming and FOV

	// FOV when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/** Automatic Fire **/
	FTimerHandle FireTimer;
	bool bCurrentlyFiring = false;

	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	/** Carrying Ammos **/

	// Carried ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingRLAmmo = 4;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingPistolAmmo = 12;
	
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingSMGAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingShotgunAmmo = 8;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingSniperAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingGrenadeLauncherAmmo = 4;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	
};
