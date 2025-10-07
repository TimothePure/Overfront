// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OFAnimInstance.h"

#include "Character/OverfrontCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UOFAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<AOverfrontCharacter>(TryGetPawnOwner());
}

void UOFAnimInstance::
NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr)
	{
		Character = Cast<AOverfrontCharacter>(TryGetPawnOwner());
	}
	if (Character == nullptr) return;

	FVector Velocity = Character->GetVelocity(); 
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = Character->IsWeaponEquipped();
	bIsCrouched = Character->bIsCrouched;
	bAiming = Character->IsAiming();
}
