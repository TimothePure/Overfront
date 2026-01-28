// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Overfront/Enums/OFCombatStates.h"
#include "Overfront/Enums/OFWeaponTypes.h"
#include "Widgets/OFHUD.h"
#include "OFCombatComponent.generated.h"


#define CAMERA_IGNORE_DISTANCE = 350.f;

class AOFProjectile;
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
	void SwapWeapons();
	
	UFUNCTION(Server, Reliable)
	void ServerSwapWeapons();
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& HitTarget);
	
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	bool bLocallyReloading = false;
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void FireInput(bool bPressed);

	void Fire();
	
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& HitResult);

	void SetHUDCrosshair(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 AmountToReload();
	
	void ThrowGrenade();
	
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AOFProjectile> GrenadeClass;
	
	void DroppedEquippedWeapon();
	void EquipPrimaryWeapon(AOFWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AOFWeapon* WeaponToEquip);
	
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBack(AActor* ActorToAttach);
	
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AOFWeapon* WeaponToEquip);
	void ReloadWeaponIfEmpty();
	
	
	void ApplyRecoil();
	void UpdateRecoil(float DeltaTime);

private:
	class AOverfrontCharacter* Character;
	class AOFPlayerController* Controller;
	class AOFHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AOFWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AOFWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;
	
	UFUNCTION()
	void OnRep_Aiming();
	
	// Local value of aiming
	bool bAimButtonPressed = false;

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
	
	UPROPERTY(EditAnywhere)
	TMap<EWeaponType, int32> MaxCarriedAmmoMap;

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
	void InitializeMaxCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	void ShowAttachedGrenade(bool bShowGrenade);
	
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;
	
	UFUNCTION()
	void OnRep_Grenades();
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	int32 MaxGrenades = 4;
	
	void UpdateHUDGrenades();

	float RecoilOffset = 0.f;

	float RecoilVelocity = 0.f;
	
public:
	FORCEINLINE float GetRecoilOffset() const { return RecoilOffset; }
	FORCEINLINE bool ShouldSwapWeapons() const { return EquippedWeapon != nullptr && SecondaryWeapon != nullptr; }
	FORCEINLINE float GetGrenades() const { return Grenades; }
};
