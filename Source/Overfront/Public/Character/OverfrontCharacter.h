// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/OFInteractWithCrosshairInterface.h"
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
	
public:
	AOverfrontCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);

	virtual void OnRep_ReplicatedMovement() override;

	void OnEliminated(float Delay);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnEliminated();

	void UpdateHUDHealth();

protected:
	virtual void BeginPlay() override;
	
	/** Jump **/
	virtual void Jump() override; 
	virtual void DoJumpStart();
	virtual void DoJumpEnd();

	/** Movement **/
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	void MoveInput(const FInputActionValue& Value);

	/** Look **/
	void LookInput(const FInputActionValue& Value);
	virtual void DoLook(float Yaw, float Pitch);

	/** Equip **/
	void EquipInput(const FInputActionValue& Value);

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
	
	void SimProxiesTurn();
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
private:
	/** Character Components **/
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	class UOFCombatComponent* CombatComponent;

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
	void OnRep_Health();

	/** Elimination **/
	void EnterRagdollState();
	bool bIsEliminated = false;
	FTimerHandle EliminationTimerHandle;
	void EliminationTimerFinished();

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	UMaterialInstance* GrayscaleMaterialInstance;

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
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
};
