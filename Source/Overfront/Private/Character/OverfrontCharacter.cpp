// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OverfrontCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Character/OFAnimInstance.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/OFBuffComponent.h"
#include "Components/OFCombatComponent.h"
#include "Components/OFLagCompensationComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModes/OverfrontGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Overfront/Overfront.h"
#include "PlayerController/OFPlayerController.h"
#include "PlayerState/OFPlayerState.h"
#include "Weapons/OFWeapon.h"
#include "Weapons/Damage/OFWeaponDamageType.h"

#pragma region ClassSetup

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
	
	BuffComponent = CreateDefaultSubobject<UOFBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);
	
	LagCompensationComponent = CreateDefaultSubobject<UOFLagCompensationComponent>(TEXT("LagCompensationComponent"));
	LagCompensationComponent->SetIsReplicated(true);
 
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);
	
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Hitboxes for server-side rewind
	{
		HeadBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBox"));
		HeadBox->SetupAttachment(GetMesh(), FName("head"));
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("head"), HeadBox);
		
		PelvisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PelvisBox"));
		PelvisBox->SetupAttachment(GetMesh(), FName("pelvis"));
		PelvisBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("pelvis"), PelvisBox);
		
		Spine02Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_02Box"));
		Spine02Box->SetupAttachment(GetMesh(), FName("spine_02"));
		Spine02Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("spine_02"), Spine02Box);
		
		Spine03Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_03Box"));
		Spine03Box->SetupAttachment(GetMesh(), FName("spine_03"));
		Spine03Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("spine_03"), Spine03Box);
		
		UpperArmLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArmLBox"));
		UpperArmLBox->SetupAttachment(GetMesh(), FName("upperarm_l"));
		UpperArmLBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("upperarm_l"), UpperArmLBox);
		
		UpperArmRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArmRBox"));
		UpperArmRBox->SetupAttachment(GetMesh(), FName("upperarm_r"));
		UpperArmRBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("upperarm_r"), UpperArmRBox);
		
		LowerArmLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArmLBox"));
		LowerArmLBox->SetupAttachment(GetMesh(), FName("lowerarm_l"));
		LowerArmLBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("lowerarm_l"), LowerArmLBox);
		
		LowerArmRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArmRBox"));
		LowerArmRBox->SetupAttachment(GetMesh(), FName("lowerarm_r"));
		LowerArmRBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("lowerarm_r"), LowerArmRBox);
		
		HandLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HandLBox"));
		HandLBox->SetupAttachment(GetMesh(), FName("hand_l"));
		HandLBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("hand_l"), HandLBox);
		
		HandRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HandRBox"));
		HandRBox->SetupAttachment(GetMesh(), FName("hand_r"));
		HandRBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("hand_r"), HandRBox);
		
		ThighLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ThighLBox"));
		ThighLBox->SetupAttachment(GetMesh(), FName("thigh_l"));
		ThighLBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("thigh_l"), ThighLBox);
		
		ThighRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ThighRBox"));
		ThighRBox->SetupAttachment(GetMesh(), FName("thigh_r"));
		ThighRBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("thigh_r"), ThighRBox);
		
		CalfLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CalfLBox"));
		CalfLBox->SetupAttachment(GetMesh(), FName("calf_l"));
		CalfLBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("calf_l"), CalfLBox);
		
		CalfRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CalfRBox"));
		CalfRBox->SetupAttachment(GetMesh(), FName("calf_r"));
		CalfRBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("calf_r"), CalfRBox);
		
		FootLBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FootLBox"));
		FootLBox->SetupAttachment(GetMesh(), FName("foot_l"));
		FootLBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("foot_l"), FootLBox);
		
		FootRBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FootRBox"));
		FootRBox->SetupAttachment(GetMesh(), FName("foot_r"));
		FootRBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBoxes.Add(FName("foot_r"), FootRBox);
	}
}

void AOverfrontCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AOverfrontCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AOverfrontCharacter, Health);
	DOREPLIFETIME(AOverfrontCharacter, Shield);
}

void AOverfrontCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
	if (BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	
	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this;
		if (Controller)
		{
			LagCompensationComponent->Controller = Cast<AOFPlayerController>(Controller);
		}
	}
}

void AOverfrontCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
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
        EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AOverfrontCharacter::ReloadInput);
        EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &AOverfrontCharacter::ThrowGrenadeInput);
        EnhancedInputComponent->BindAction(SwapWeaponsAction, ETriggerEvent::Started, this, &AOverfrontCharacter::SwapWeaponsInput);
	}
}

void AOverfrontCharacter::SpawnDefaultWeapon()
{
	AOverfrontGameMode* GameMode = Cast<AOverfrontGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (GameMode && World && DefaultWeaponClass && !bIsEliminated)
	{
		AOFWeapon* StartingWeapon = World->SpawnActor<AOFWeapon>(DefaultWeaponClass);
		if (CombatComponent)
		{
			CombatComponent->EquipWeapon(StartingWeapon);
			StartingWeapon->bDestroyWeapon = true;
		}
	}
}

void AOverfrontCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentSensitivity = MouseSensitivity;
	
	// Called inside BeginPlay and Controller->OnRep_PlayerState to ensure that the HUD is setup correctly
	if (AOFPlayerState* PS = GetPlayerState<AOFPlayerState>())
	{
		PS->AddToScore(0.f);
		PS->AddToDefeats(0);
	}
	
	UpdateHUDHealth();
	UpdateHUDShield();
	UpdateHUDWeapon();
	
	if (OFPlayerController)
	{
		OFPlayerController->SetWeaponHUDVisibility(false);
	}
	
	if (HasAuthority())
	{
		OnTakePointDamage.AddDynamic(this, &AOverfrontCharacter::HandlePointDamage);
		OnTakeRadialDamage.AddDynamic(this, &AOverfrontCharacter::HandleRadialDamage);
	}
	
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
	
	SpawnDefaultWeapon();
}

void AOverfrontCharacter::Destroyed()
{
	Super::Destroyed();

	AOverfrontGameMode* OverfrontGameMode = Cast<AOverfrontGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = OverfrontGameMode && OverfrontGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquippedWeapon->Destroy();
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
		AddControllerYawInput(Yaw * CurrentSensitivity);
		AddControllerPitchInput(Pitch * CurrentSensitivity);
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

FRotator AOverfrontCharacter::GetViewRotation() const
{
	FRotator View = Super::GetViewRotation();

	if (CombatComponent)
	{
		View.Pitch += CombatComponent->GetRecoilOffset();
	}

	return View;
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

void AOverfrontCharacter::SwapWeaponsInput(const FInputActionValue& Value)
{
	if (CombatComponent && CombatComponent->ShouldSwapWeapons())
	{
		CombatComponent->SwapWeapons();
	}
}

void AOverfrontCharacter::ServerEquip_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AOverfrontCharacter::ReloadInput(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void AOverfrontCharacter::ThrowGrenadeInput(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->ThrowGrenade();
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
	if (!IsLocallyControlled()) return;
	
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

void AOverfrontCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("RocketLauncher");
				break;
			case EWeaponType::EWT_Pistol:
				SectionName = FName("Pistol");
				break;
			case EWeaponType::EWT_SubmachineGun:
				SectionName = FName("Pistol");
				break;  
			case EWeaponType::EWT_Shotgun:
				SectionName = FName("Shotgun");
				break;
			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("Sniper");
				break;
			case EWeaponType::EWT_GrenadeLauncher:
				SectionName = FName("GrenadeLauncher");
				break;
		}

		AnimInstance->Montage_JumpToSection(SectionName, ReloadMontage);
	}
}

void AOverfrontCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void AOverfrontCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	float DamageRemaining = Damage;
	
	if (Shield > 0.f)
	{
		const float ShieldDamage = FMath::Min(Shield, DamageRemaining);
		Shield = FMath::Clamp(Shield - ShieldDamage, 0.f, MaxShield);
		DamageRemaining -= ShieldDamage;
	}
	
	if (DamageRemaining > 0.f)
	{
		Health = FMath::Clamp(Health - DamageRemaining, 0.f, MaxHealth);
	}
	
	if (IsLocallyControlled())
	{
		UpdateHUDHealth();
		UpdateHUDShield();
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

void AOverfrontCharacter::HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatorController, FVector HitLocation,
	UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	const UOFWeaponDamageType* WeaponDamageType = Cast<UOFWeaponDamageType>(DamageType);
	
	if (!WeaponDamageType) return;
	
	float FinalDamage = WeaponDamageType->BaseDamage;
	
	// Hit zone detection
	if (BoneName == FName("head"))
	{
		FinalDamage = WeaponDamageType->HeadDamage;
	}
	else if (BoneName == FName("pelvis") || BoneName.ToString().Contains("spine"))
	{
		FinalDamage = WeaponDamageType->TorsoDamage;
	}
	else
	{
		FinalDamage = WeaponDamageType->LimbsDamage;
	}
	
	ReceiveDamage(DamagedActor, FinalDamage, DamageType, InstigatorController, DamageCauser);
}

void AOverfrontCharacter::HandleRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	FVector Origin, const FHitResult& HitInfo, AController* InstigatorController, AActor* DamageCauser)
{
	ReceiveDamage(DamagedActor, Damage, DamageType, InstigatorController, DamageCauser);
}


// Called by the GameMode so only on the server
void AOverfrontCharacter::OnEliminated(float Delay)
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DropOrDestroyWeapons();
	MulticastOnEliminated();
	if (CombatComponent)
	{
		CombatComponent->FireInput(false);
	}
	GetWorldTimerManager().SetTimer(EliminationTimerHandle, this, &ThisClass::EliminationTimerFinished, Delay);
}

void AOverfrontCharacter::MulticastOnEliminated_Implementation()
{
	if (OFPlayerController)
	{
		OFPlayerController->SetHUDWeaponAmmo(0);
		OFPlayerController->SetWeaponHUDVisibility(false);
	}
	EnterRagdollState();
	if (IsLocallyControlled())
	{
		FollowCamera->PostProcessSettings.AddBlendable(GrayscaleMaterialInstance, 1.0f);
	}
	if (IsLocallyControlled() && CombatComponent->bAiming && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

void AOverfrontCharacter::EliminationTimerFinished()
{
	if (AOverfrontGameMode* OverfrontGameMode = GetWorld()->GetAuthGameMode<AOverfrontGameMode>())
	{
		OverfrontGameMode->RequestRespawn(this, Controller);
	}
}

void AOverfrontCharacter::DropOrDestroyWeapon(AOFWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	} else
	{
		Weapon->Dropped();
	}
}

void AOverfrontCharacter::DropOrDestroyWeapons()
{
	if (CombatComponent == nullptr) return;
	
	if (CombatComponent->EquippedWeapon)
	{
		DropOrDestroyWeapon(CombatComponent->EquippedWeapon);
		CombatComponent->EquippedWeapon = nullptr;
	}
	if (CombatComponent->SecondaryWeapon)
	{
		DropOrDestroyWeapon(CombatComponent->SecondaryWeapon);
		CombatComponent->SecondaryWeapon = nullptr;
	}
	
	SetOverlappingWeapon(nullptr);
}

#pragma endregion Combat

#pragma region PlayerStats

void AOverfrontCharacter::OnRep_Health(float LastHealth)
{
	if (IsLocallyControlled())
	{
		UpdateHUDHealth();
	}
	
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void AOverfrontCharacter::OnRep_Shield(float LastShield)
{
	if (IsLocallyControlled())
	{
		UpdateHUDShield();
	}
	
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void AOverfrontCharacter::UpdateHUDHealth()
{
	OFPlayerController = OFPlayerController == nullptr ? Cast<AOFPlayerController>(GetController()) : OFPlayerController;
	if (OFPlayerController)
	{
		OFPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AOverfrontCharacter::UpdateHUDShield()
{
	OFPlayerController = OFPlayerController == nullptr ? Cast<AOFPlayerController>(GetController()) : OFPlayerController;
	if (OFPlayerController)
	{
		OFPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void AOverfrontCharacter::UpdateHUDWeapon()
{
	OFPlayerController = OFPlayerController == nullptr ? Cast<AOFPlayerController>(GetController()) : OFPlayerController;
	if (OFPlayerController && CombatComponent)
	{
		OFPlayerController->SetWeaponHUDVisibility(GetEquippedWeapon() != nullptr);
		if (GetEquippedWeapon())
		{
			OFPlayerController->SetHUDWeaponType(GetEquippedWeapon()->GetWeaponType());
		}
		OFPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
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

ECombatState AOverfrontCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX;

	return CombatComponent->CombatState;
}

bool AOverfrontCharacter::IsLocallyReloading()
{
	if (!CombatComponent) return false;
	return CombatComponent->bLocallyReloading;
}

float AOverfrontCharacter::GetCarriedAmmo()
{
	if (CombatComponent == nullptr) return -1;
	return CombatComponent->CarriedAmmo;
}
#pragma endregion Getters

