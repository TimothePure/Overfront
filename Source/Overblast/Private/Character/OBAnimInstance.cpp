// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OBAnimInstance.h"

#include "Character/OBCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UOBAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<AOBCharacter>(TryGetPawnOwner());
}

void UOBAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr)
	{
		Character = Cast<AOBCharacter>(TryGetPawnOwner());
	}
	if (Character == nullptr) return;

	FVector Velocity = Character->GetVelocity(); 
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
