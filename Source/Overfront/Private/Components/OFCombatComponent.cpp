// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OFCombatComponent.h"

#include <filesystem>

#include "Camera/CameraComponent.h"
#include "Character/OverfrontCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/OFPlayerController.h"
#include "Weapons/OFHitScanWeapon.h"
#include "Weapons/OFProjectile.h"
#include "Weapons/OFShotgun.h"
#include "Weapons/OFWeapon.h"
#include "Widgets/OFHUD.h"


UOFCombatComponent::UOFCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 400.0f;
	
	// To fill the MaxCarriedAmmo TMap for being able to adjust it from the editor
	InitializeMaxCarriedAmmo();
}

void UOFCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UOFCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UOFCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UOFCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UOFCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UOFCombatComponent, CombatState);
	DOREPLIFETIME(UOFCombatComponent, Grenades);
}

void UOFCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmoMap[WeaponType]);
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UOFCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (UCameraComponent* Camera = Character->GetFollowCamera())
		{
			DefaultFOV = Camera->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
	UpdateHUDGrenades();
}  

void UOFCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		Target = HitResult.ImpactPoint;

		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
		UpdateRecoil(DeltaTime);
	}
}

#pragma region FiringWeapon

void UOFCombatComponent::FireInput(bool bPressed)
{
	bFireInputPressed = bPressed;
	
	if (bFireInputPressed)
	{
		Fire();
	}
}

void UOFCombatComponent::Fire()
{
	if (CanFire())
	{
		bCurrentlyFiring = true;

		switch (EquippedWeapon->FireType)
		{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
		}
		
		CrosshairShootingFactor = 0.75f;
		StartFireTimer();
		ApplyRecoil();
	}
}

void UOFCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		Target = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(Target) : Target;
		if (!Character->HasAuthority())
		{
			LocalFire(Target);
		}
		ServerFire(Target);
	}
}

void UOFCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		Target = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(Target) : Target;
		if (!Character->HasAuthority())
		{
			LocalFire(Target);
		}
		ServerFire(Target);
	}
}

void UOFCombatComponent::FireShotgun()
{
	if (AOFShotgun* ShotgunWeapon = Cast<AOFShotgun>(EquippedWeapon))
	{
		TArray<FVector_NetQuantize> HitTargets;
		ShotgunWeapon->ShotgunTraceEndWithScatter(Target, HitTargets);
		if (Character && !Character->HasAuthority())
		{
			LocalShotgunFire(HitTargets);
		}
		ServerShotgunFire(HitTargets);
	}
}

void UOFCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UOFCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCurrentlyFiring = false;
	if (bFireInputPressed && EquippedWeapon->bAutomaticFire)
	{
		Fire();
	}
	ReloadWeaponIfEmpty();
}

void UOFCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	
	if (CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UOFCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AOFShotgun* ShotgunWeapon = Cast<AOFShotgun>(EquippedWeapon);
	if (ShotgunWeapon == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		ShotgunWeapon->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}	
}

void UOFCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UOFCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UOFCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UOFCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalShotgunFire(TraceHitTargets);
}

bool UOFCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	if (bLocallyReloading) return false;
	if (!EquippedWeapon->IsEmpty() && !bCurrentlyFiring && CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	return !EquippedWeapon->IsEmpty() && !bCurrentlyFiring && CombatState == ECombatState::ECS_Unoccupied;
}

#pragma endregion FiringWeapon

#pragma region EquippingWeapon

void UOFCombatComponent::EquipWeapon(AOFWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr || CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	} else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UOFCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	ServerSwapWeapons();
}

void UOFCombatComponent::ServerSwapWeapons_Implementation()
{
	if (!EquippedWeapon || !SecondaryWeapon) return;
	
	AOFWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	// For Primary Weapon
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->SetHUDWeaponType();
	PlayEquipWeaponSound(EquippedWeapon);
	UpdateCarriedAmmo();
	ReloadWeaponIfEmpty();

	// For secondary Weapon
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBack(SecondaryWeapon);
	SecondaryWeapon->SetOwner(Character);
}

void UOFCombatComponent::EquipPrimaryWeapon(AOFWeapon* WeaponToEquip)
{
	DroppedEquippedWeapon();
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	AttachActorToRightHand(EquippedWeapon);
	
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->SetHUDWeaponType();

	if (Controller)
	{
		Controller->SetWeaponHUDVisibility(EquippedWeapon != nullptr);
	}

	UpdateCarriedAmmo();
	PlayEquipWeaponSound(WeaponToEquip);
	ReloadWeaponIfEmpty();
}

void UOFCombatComponent::EquipSecondaryWeapon(AOFWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	SecondaryWeapon->SetOwner(Character);
	AttachActorToBack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);
}

void UOFCombatComponent::OnRep_EquippedWeapon()
{
	if (Controller)
	{
		Controller->SetWeaponHUDVisibility(EquippedWeapon != nullptr);
	}
	
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo(); 
	}
}

void UOFCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBack(SecondaryWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UOFCombatComponent::DroppedEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UOFCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UOFCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolLeftHandSocket") : FName("LeftHandSocket");
	
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName))
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UOFCombatComponent::AttachActorToBack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	bool bUsePistolSocket = SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("SecondaryPistolSocket") : FName("SecondaryRifleSocket");

	if (const USkeletalMeshSocket* SecondarySocket = Character->GetMesh()->GetSocketByName(SocketName))
	{
		SecondarySocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UOFCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AOFPlayerController>(Character->GetController()) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UOFCombatComponent::PlayEquipWeaponSound(AOFWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
	}
}

void UOFCombatComponent::ReloadWeaponIfEmpty()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

#pragma endregion EquippingWeapon

#pragma region Aiming

void UOFCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
		Character->CurrentSensitivity = Character->MouseSensitivity * (bIsAiming ? Character->SniperSensitivityMultiplier : 1);
	}
	
	if (Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UOFCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UOFCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition, CrosshairWorldDirection;
	
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = Character->GetSpringArm()->TargetArmLength;
			Start += CrosshairWorldDirection * (DistanceToCharacter);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}
	}

	if (HitResult.GetActor() && HitResult.GetActor()->Implements<UOFInteractWithCrosshairInterface>())
	{
		HUDPackage.CrosshairColor = FLinearColor::Red;
	} else
	{
		HUDPackage.CrosshairColor = FLinearColor::White;
	}
}

void UOFCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	
	Controller = Controller == nullptr ? Cast<AOFPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AOFHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
			} else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr; 
			}

			// Calculate crosshair spread
			FVector2D MaxSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRangeAiming(0.f, 1.f);
			FVector2D VelocityMultiplierRange(0.3f, 1.f);
			
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
				MaxSpeedRange,
				bAiming ? VelocityMultiplierRangeAiming : VelocityMultiplierRange,
				bAiming ? Velocity.Size() / 3 : Velocity.Size());
			
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			} else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UOFCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	} else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

#pragma endregion Aiming

#pragma region Ammo

void UOFCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AOFPlayerController>(Character->GetController()) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	if (CombatState == ECombatState::ECS_Reloading && EquippedWeapon != nullptr && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UOFCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRLAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UOFCombatComponent::InitializeMaxCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, 200);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, 20);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, 120);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, 200);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, 60);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, 80);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, 60);
}

void UOFCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied)
	{
		if (EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
		{
			ServerReload();
			HandleReload();
			bLocallyReloading = true;
		}
	}
}

void UOFCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled())
	{
		HandleReload();
	}
}

void UOFCombatComponent::HandleReload()
{
	if (Character == nullptr) return;
	Character->PlayReloadMontage();
}

int32 UOFCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least =  FMath::Min(AmountCarried, RoomInMag);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UOFCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReloading = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireInputPressed)
	{
		Fire();
	}
}

void UOFCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UOFCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];

		Controller = Controller == nullptr ? Cast<AOFPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);
		}
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UOFCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];

		Controller = Controller == nullptr ? Cast<AOFPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);
		}
	}
	EquippedWeapon->AddAmmo(1);
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UOFCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

#pragma endregion Ammo

#pragma region Grenade 

void UOFCombatComponent::ThrowGrenade()
{
	if (Grenades == 0 || CombatState != ECombatState::ECS_Unoccupied) return;
	
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
		
		if (!Character->HasAuthority())
		{
			ServerThrowGrenade();
		} else
		{
			Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
			UpdateHUDGrenades();
		}
	}
		
}

void UOFCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return; 
	
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UOFCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
} 

void UOFCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(Target);
	}
}

void UOFCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& HitTarget)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = HitTarget - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		
		if (UWorld* World = GetWorld())
		{
			World->SpawnActor<AOFProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
		}
	}
}

void UOFCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<AOFPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UOFCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UOFCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

#pragma endregion Grenade

void UOFCombatComponent::ApplyRecoil()
{
	if (!Character || !EquippedWeapon) return;

	RecoilVelocity += EquippedWeapon->RecoilImpulse;
}

void UOFCombatComponent::UpdateRecoil(float DeltaTime)
{
	if (!Character || !Character->IsLocallyControlled() || !EquippedWeapon) return;

	const float Stiffness = EquippedWeapon->RecoilSpringStrength;
	const float Damping   = EquippedWeapon->RecoilDamping;

	RecoilVelocity += (-RecoilOffset * Stiffness - RecoilVelocity * Damping) * DeltaTime;
	RecoilOffset += RecoilVelocity * DeltaTime;
 }

void UOFCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UOFCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
		case ECombatState::ECS_Reloading:
			if (Character && !Character->IsLocallyControlled())
			{
				HandleReload();
			}
			break;
		case ECombatState::ECS_Unoccupied:
			if (bFireInputPressed)
			{
				Fire();
			}
			break;
		case ECombatState::ECS_ThrowingGrenade:
			if (Character && !Character->IsLocallyControlled())
			{
				Character->PlayThrowGrenadeMontage();
				AttachActorToLeftHand(EquippedWeapon);
				ShowAttachedGrenade(true);
			}
			break;
		default: break;
	}
}
