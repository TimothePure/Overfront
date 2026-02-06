// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/OFCombatComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/OFInteractWithCrosshairInterface.h"
#include "Overfront/Enums/OFCombatStates.h"
#include "Overfront/Enums/OFTurningInPlace.h"
#include "OverfrontCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class OVERFRONT_API AOverfrontCharacter : public ACharacter, public IOFInteractWithCrosshairInterface
{
	GENERATED_BODY()

	/** Default Input Mapping Context **/
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* DefaultMappingContext;

	/** Combat Input Mapping Context **/
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* CombatMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	/** Equip Weapon Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* EquipAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AimAction;

	/** Fire Weapon Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* FireWeaponAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ReloadAction;
	
	/** Throw Grenade Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ThrowGrenadeAction;

	/** Swap Weapons Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SwapWeaponsAction;
	
public:
	AOverfrontCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	
	void PlayThrowGrenadeMontage();

	virtual void OnRep_ReplicatedMovement() override;

	void OnEliminated(float Delay);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnEliminated();

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDWeapon();	
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitBoxes;
	
	UPROPERTY(EditAnywhere, Category = "PlayerSettings")
	float MouseSensitivity = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "PlayerSettings")
	float SniperSensitivityMultiplier = 0.4f;

	UPROPERTY()
	float CurrentSensitivity;
	
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
	/** Jump **/
	virtual void Jump() override; 
	virtual void DoJumpStart();
	virtual void DoJumpEnd();

	virtual FRotator GetViewRotation() const override;

	/** Movement **/
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	void MoveInput(const FInputActionValue& Value);

	/** Look **/
	void LookInput(const FInputActionValue& Value);
	virtual void DoLook(float Yaw, float Pitch);

	/** Equip **/
	void EquipInput(const FInputActionValue& Value);

	/** Swap Weapons **/
	void SwapWeaponsInput(const FInputActionValue& Value);

	/** Crouch **/
	void CrouchInputStart(const FInputActionValue& Value);
	void CrouchInputStop(const FInputActionValue& Value);
	
	/** Aim **/
	void AimInputStart(const FInputActionValue& Value);
	void AimInputEnd(const FInputActionValue& Value);
	void AimOffset(float DeltaTime);

	/** Fire **/
	void FireInputStart(const FInputActionValue& Value);
	void FireInputEnd(const FInputActionValue& Value);

	/** Reload **/
	void ReloadInput(const FInputActionValue& Value);
	
	/** Throw Grenade **/
	void ThrowGrenadeInput(const FInputActionValue& Value);
	
	void SimProxiesTurn();
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	
	void DropOrDestroyWeapon(AOFWeapon* Weapon);
	void DropOrDestroyWeapons();
	
	/** Hit boxes used for server-side rewind **/
	UPROPERTY(EditAnywhere)
	UBoxComponent* HeadBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* PelvisBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine02Box;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine03Box;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmLBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmRBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmLBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmRBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* HandLBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* HandRBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighRBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfLBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfRBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* FootLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* FootRBox;
	
private:
	/** Character Components **/
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UWidgetComponent* OverheadWidget;

	/** Character components **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UOFCombatComponent* CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UOFBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UOFLagCompensationComponent* LagCompensationComponent;
	
	UPROPERTY()
	class AOFPlayerController* OFPlayerController;
	
	/** Weapon Pickup **/
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AOFWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AOFWeapon* LastWeapon) const;
	
	UFUNCTION(Server, Reliable)
	void ServerEquip();

	/** Aiming properties **/
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/** Animation Montages **/
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ThrowGrenadeMontage;

	/** Hiding Character from Camera **/
	void HideCharacterIfCameraClose();

	UPROPERTY(EditAnywhere, Category = "Camera")
	float PlayerHideThresold = 200.f;

	/** Rotation Movement Replication **/
	bool bRotateRootBone;
	float TurnThresold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	void CalculateAO_Pitch();
	float CalculateSpeed();
	
	/** Player Health **/
	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category="Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);
	
	/** Player Shield **/
	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Shield, Category="Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	/** Elimination **/
	void EnterRagdollState();
	bool bIsEliminated = false;
	FTimerHandle EliminationTimerHandle;
	void EliminationTimerFinished();

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	UMaterialInstance* GrayscaleMaterialInstance;

	/** Grenade **/
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* AttachedGrenade;
	
	/** Default Weapon **/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AOFWeapon> DefaultWeaponClass;
public:
	/** Getters and Setters **/
	void SetOverlappingWeapon(AOFWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }
	AOFWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }	
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }	
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatState() const;
	FORCEINLINE UOFCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UOFBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	bool IsLocallyReloading();
	FORCEINLINE UOFLagCompensationComponent* GetLagCompensationComponent() { return LagCompensationComponent; }
	float GetCarriedAmmo();
};
