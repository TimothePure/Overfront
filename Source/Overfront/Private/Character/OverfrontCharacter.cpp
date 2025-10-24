// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OverfrontCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Character/OFAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/OFCombatComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModes/OverfrontGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Overfront/Overfront.h"
#include "PlayerController/OFPlayerController.h"
#include "PlayerState/OFPlayerState.h"
#include "Weapons/OFWeapon.h"

#pragma region ClassSetup

class AOverfrontGameMode;

AOverfrontCharacter::AOverfrontCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UOFCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);
}

void AOverfrontCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AOverfrontCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AOverfrontCharacter, Health);
}

void AOverfrontCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void AOverfrontCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AOverfrontCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOverfrontCharacter::DoJumpEnd);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOverfrontCharacter::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOverfrontCharacter::LookInput);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AOverfrontCharacter::EquipInput);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AOverfrontCharacter::CrouchInputStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AOverfrontCharacter::CrouchInputStop);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AOverfrontCharacter::AimInputStart);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AOverfrontCharacter::AimInputEnd);
		EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &AOverfrontCharacter::FireInputStart);
        EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &AOverfrontCharacter::FireInputEnd);
	}
}

void AOverfrontCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	OFPlayerController = Cast<AOFPlayerController>(GetController());
	if (OFPlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OFPlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			if (CombatMappingContext)
			{
				Subsystem->AddMappingContext(CombatMappingContext, 0);
			}
		}
	}

	// Called inside BeginPlay and Controller->OnRep_PlayerState to ensure that the HUD is setup correctly
	if (AOFPlayerState* PS = GetPlayerState<AOFPlayerState>())
	{
		PS->AddToScore(0.f);
	}
	
	UpdateHUDHealth();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AOverfrontCharacter::ReceiveDamage);
	}
}

void AOverfrontCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	} else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCharacterIfCameraClose();
}

#pragma endregion ClassSetup

#pragma region CharacterMovement

void AOverfrontCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	Super::Jump();  
}

void AOverfrontCharacter::MoveInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AOverfrontCharacter::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AOverfrontCharacter::CrouchInputStart(const FInputActionValue& Value)
{
	Crouch();
}

void AOverfrontCharacter::CrouchInputStop(const FInputActionValue& Value)
{
	UnCrouch();
}

void AOverfrontCharacter::EnterRagdollState()
{
	FVector CurrentVelocity = GetCharacterMovement()->Velocity;
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (OFPlayerController)
	{
		DisableInput(OFPlayerController);
	}
	
	// Disable collision and set ragdoll state
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->SetAllPhysicsLinearVelocity(CurrentVelocity);
	GetMesh()->AddImpulseToAllBodiesBelow(FVector(0.f, 0.f, 50.f), TEXT("pelvis"), true);
}

void AOverfrontCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AOverfrontCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AOverfrontCharacter::DoJumpStart()
{
	Jump();
}
 
void AOverfrontCharacter::DoJumpEnd()
{
	StopJumping();
}

void AOverfrontCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

#pragma endregion CharacterMovement

#pragma region Combat

void AOverfrontCharacter::AimInputStart(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void AOverfrontCharacter::AimInputEnd(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void AOverfrontCharacter::FireInputStart(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->FireInput(true);
	}
}

void AOverfrontCharacter::FireInputEnd(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->FireInput(false);
	}
}

void AOverfrontCharacter::EquipInput(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		} else
		{
			ServerEquip();
		}
	}
}

void AOverfrontCharacter::ServerEquip_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AOverfrontCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < PlayerHideThresold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	} else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AOverfrontCharacter::SetOverlappingWeapon(AOFWeapon* Weapon)
{
	// Hide previous weapon widget
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	
	OverlappingWeapon = Weapon;

	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

void AOverfrontCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float AOverfrontCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity(); 
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AOverfrontCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && !CombatComponent->EquippedWeapon) return;
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (CalculateSpeed() == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	} else // Running or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void AOverfrontCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;

	if (CalculateSpeed() > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThresold)
	{
		if (ProxyYaw > TurnThresold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		} else if (ProxyYaw < -TurnThresold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		} else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AOverfrontCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	} else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		}
	}
}

void AOverfrontCharacter::OnRep_OverlappingWeapon(AOFWeapon* LastWeapon) const
{
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
	
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

void AOverfrontCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleIronsights") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AOverfrontCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AOverfrontCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	class AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	if (IsLocallyControlled())
	{
		UpdateHUDHealth();
	}
	PlayHitReactMontage();

	if (Health <= 0.f)
	{
		if (AOverfrontGameMode* OverfrontGameMode = GetWorld()->GetAuthGameMode<AOverfrontGameMode>())
		{
			OFPlayerController = OFPlayerController == nullptr ? Cast<AOFPlayerController>(Controller) : OFPlayerController;
			OverfrontGameMode->PlayerEliminated(this, OFPlayerController, Cast<AOFPlayerController>(InstigatorController));
		}
	}
}

// Called by the GameMode so only on the server
void AOverfrontCharacter::OnEliminated()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}
	MulticastOnEliminated();
	GetWorldTimerManager().SetTimer(EliminationTimerHandle, this, &ThisClass::EliminationTimerFinished, EliminationDelay);
}

void AOverfrontCharacter::MulticastOnEliminated_Implementation()
{
	EnterRagdollState();
}

void AOverfrontCharacter::EliminationTimerFinished()
{
	if (AOverfrontGameMode* OverfrontGameMode = GetWorld()->GetAuthGameMode<AOverfrontGameMode>())
	{
		OverfrontGameMode->RequestRespawn(this, Controller);
	}
}

#pragma endregion Combat

#pragma region PlayerStats

void AOverfrontCharacter::OnRep_Health()
{
	if (IsLocallyControlled())
	{
		UpdateHUDHealth();
	}
	PlayHitReactMontage();
}

void AOverfrontCharacter::UpdateHUDHealth()
{
	OFPlayerController = OFPlayerController == nullptr ? Cast<AOFPlayerController>(GetController()) : OFPlayerController;
	if (OFPlayerController)
	{
		OFPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

#pragma endregion PlayerStats

#pragma region Getters
bool AOverfrontCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool AOverfrontCharacter::IsAiming()
{
	return  (CombatComponent && CombatComponent->bAiming);
}

AOFWeapon* AOverfrontCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}

FVector AOverfrontCharacter::GetHitTarget() const
{
	if (CombatComponent == nullptr) return FVector(0, 0, 0);

	return CombatComponent->Target;
}
#pragma endregion Getters

