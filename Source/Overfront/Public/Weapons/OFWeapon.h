// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Overfront/Enums/OFWeaponTypes.h"
#include "OFWeapon.generated.h"

class UOFWeaponDamageType;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class OVERFRONT_API AOFWeapon : public AActor
{
	GENERATED_BODY()

public:
	AOFWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void SetHUDWeaponType();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 Amount);

	// Textures for the weapon crosshairs
	UPROPERTY(EditAnywhere, Category = "Weapon|Crosshair")
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Weapon|Crosshair")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Weapon|Crosshair")
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "Weapon|Crosshair")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Weapon|Crosshair")
	UTexture2D* CrosshairBottom;

	// Zoomed FOV while aiming
	UPROPERTY(EditAnywhere, Category = "Weapon|Aiming")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Weapon|Aiming")
	float ZoomInterpSpeed = 20.f;

	// Automatic Fire properties
	UPROPERTY(EditAnywhere, Category = "Weapon|Automatic Fire")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Weapon|Automatic Fire")
	bool bAutomaticFire = true;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Sounds")
	USoundBase* EquipSound;
	
	/** Enable or disable custom depth **/
	void EnableCustomDepth(bool bEnable);
	
	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere, Category = "Weapon|Recoil")
	float RecoilSpringStrength = 90.f;

	UPROPERTY(EditAnywhere, Category = "Weapon|Recoil")
	float RecoilDamping = 18.f;

	UPROPERTY(EditAnywhere, Category = "Weapon|Recoil")
	float RecoilImpulse = 45.f;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	
	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Damage")
	TSubclassOf<UDamageType> DamageType;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Properties")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Properties", ReplicatedUsing=OnRep_WeaponState)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon|Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon|Properties")
	TSubclassOf<class AOFBulletShell> BulletShellClass;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "Weapon|Properties")
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendAmmo();
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Properties")
	int32 MagCapacity;

	UPROPERTY()
	class AOverfrontCharacter* OwnerCharacter;

	UPROPERTY()
	class AOFPlayerController* OwnerPlayerController;

	UPROPERTY(EditAnywhere, Category = "Weapon|Properties")
	EWeaponType WeaponType;
	
public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty() const;
	bool IsFull() const;
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
