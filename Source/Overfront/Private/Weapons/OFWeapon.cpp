// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFWeapon.h"

#include "Character/OverfrontCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"


AOFWeapon::AOFWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// On the clients we don't want to collide with the weapon sphere
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AOFWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOFWeapon, WeaponState);
}

void AOFWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
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
// On the server
void AOFWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		// case EWeaponState::EWS_Initial:
		// 	ShowPickupWidget(true);
		// 	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 	break;
		// case EWeaponState::EWS_Dropped:
		// 	ShowPickupWidget(true);
		// 	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 	break;
		// default: break;
	}
	
}
// On the client
void AOFWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			break;
		// case EWeaponState::EWS_Initial:
		// 	ShowPickupWidget(true);
		// 	break;
		// case EWeaponState::EWS_Dropped:
		// 	ShowPickupWidget(true);
		// 	break;
		// default: break;
	}
}

