// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFWeapon.h"

#include "Character/OverfrontCharacter.h"
#include "Components/OFCombatComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/OFPlayerController.h"
#include "Weapons/OFBulletShell.h"

AOFWeapon::AOFWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	// On the clients we don't want to collide with the weapon sphere
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AOFWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOFWeapon, WeaponState);
	DOREPLIFETIME(AOFWeapon, Ammo);
}

void AOFWeapon::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AOverfrontCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AOFPlayerController>(OwnerCharacter->GetController()) : OwnerPlayerController;
		if (OwnerPlayerController)
		{
			OwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AOFWeapon::SetHUDWeaponType()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AOverfrontCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		OwnerPlayerController = OwnerPlayerController == nullptr ? Cast<AOFPlayerController>(OwnerCharacter->GetController()) : OwnerPlayerController;
		if (OwnerPlayerController)
		{
			if (WeaponState == EWeaponState::EWS_Dropped)
			{
				OwnerPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
			} else
			{
				OwnerPlayerController->SetHUDWeaponType(WeaponType);
			}
		}
	}
}

void AOFWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerPlayerController = nullptr;
	} else
	{
		SetHUDAmmo();
		SetHUDWeaponType();
	}
}

void AOFWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AOFWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (BulletShellClass)
	{
		if (const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject")))
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());
			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<AOFBulletShell>(BulletShellClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	
	SpendAmmo();
}

void AOFWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetHUDWeaponType();
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerPlayerController = nullptr;
}

void AOFWeapon::AddAmmo(int32 Amount)
{
	Ammo = FMath::Clamp(Ammo + Amount, 0, MagCapacity);
	SetHUDAmmo();
}

void AOFWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AOFWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	// If we are on the server, we enable the overlap collision with the pawns for the area sphere
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlapBegin);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereOverlapEnd);
	}
}

void AOFWeapon::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AOverfrontCharacter* OBCharacter = Cast<AOverfrontCharacter>(OtherActor))
	{
		OBCharacter->SetOverlappingWeapon(this);
	}
}

void AOFWeapon::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AOverfrontCharacter* OBCharacter = Cast<AOverfrontCharacter>(OtherActor))
	{
		OBCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AOFWeapon::SpendAmmo()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AOFWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
	if (OwnerCharacter && OwnerCharacter->GetCombatComponent() && IsFull())
	{
		 OwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
}

// On the server
void AOFWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
			if (WeaponType == EWeaponType::EWT_SubmachineGun)
			{
				WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				WeaponMesh->SetEnableGravity(true);
				WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			}
			EnableCustomDepth(false);
				break;
		// case EWeaponState::EWS_Initial:
		// 	ShowPickupWidget(true);
		// 	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 	break;
		case EWeaponState::EWS_Dropped:
			if (HasAuthority())
			{
				GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
			WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			ShowPickupWidget(true);
			EnableCustomDepth(true);
			break;
		default: break;
	}
}

// On the client
void AOFWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);

			if (WeaponType == EWeaponType::EWT_SubmachineGun)
			{
				WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				WeaponMesh->SetEnableGravity(true);
				WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			}
			EnableCustomDepth(false);
			break;
		// case EWeaponState::EWS_Initial:
		// 	ShowPickupWidget(true);
		// 	break;
		case EWeaponState::EWS_Dropped:
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
			WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			ShowPickupWidget(true);
			EnableCustomDepth(true);
			break;
		default: break;
	}
}

bool AOFWeapon::IsEmpty() const
{
	return Ammo <= 0;
}

bool AOFWeapon::IsFull() const
{
	return Ammo == MagCapacity;
}

