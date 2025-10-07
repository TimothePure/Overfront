// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OFCombatComponent.h"

#include "Character/OverfrontCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/OFWeapon.h"


UOFCombatComponent::UOFCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UOFCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UOFCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
}

void UOFCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming; 
}

void UOFCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UOFCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UOFCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UOFCombatComponent, bAiming);
}

void UOFCombatComponent::EquipWeapon(AOFWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
}
