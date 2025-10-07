// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OverfrontCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class OVERFRONT_API AOverfrontCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOverfrontCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category="Input")
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);

	void EquipInput(const FInputActionValue& Value);
	void CrouchInput(const FInputActionValue& Value);
	void AimInputStart(const FInputActionValue& Value);
	void AimInputEnd(const FInputActionValue& Value);

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
	

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AOFWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AOFWeapon* LastWeapon) const;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	class UOFCombatComponent* CombatComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquip();
	
public:
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	void SetOverlappingWeapon(AOFWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
};
