// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OFBuffComponent.h"

#include "Character/OverfrontCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UOFBuffComponent::UOFBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOFBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOFBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UOFBuffComponent::SetInitialJumpVelocity(float JumpZVelocity)
{
	InitialJumpVelocity = JumpZVelocity;
}

void UOFBuffComponent::Heal(float HealAmount, float HealingDuration, float TickRate)
{
	StopHealing();
	
	HealAmountTotal = HealAmount;
	HealDuration = HealingDuration;
	HealTickRate = TickRate;

	const float TickCount = HealingDuration / TickRate;

	HealAmountPerTick = HealAmount / TickCount;

	GetWorld()->GetTimerManager().SetTimer(HealTimerHandle, this, &UOFBuffComponent::ApplyHealingTick, TickRate, true);
}

void UOFBuffComponent::ApplyHealingTick()
{
	if (Character == nullptr) return;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealAmountPerTick, 0.f, Character->GetMaxHealth()));

	HealAmountTotal -= HealAmountPerTick;
	Character->UpdateHUDHealth();

	if (HealAmountTotal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		HealAmountTotal = 0.f;
		StopHealing();
	}
}

void UOFBuffComponent::StopHealing()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(HealTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);
	}
}

void UOFBuffComponent::StartSpeedBuff(float BaseSpeed, float CrouchSpeed, float Duration)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	MulticastSpeedBuff(BaseSpeed, CrouchSpeed);
	GetWorld()->GetTimerManager().SetTimer(SpeedBuffTimerHandle, this, &UOFBuffComponent::SpeedBuffFinished, Duration, true);
}

void UOFBuffComponent::SpeedBuffFinished()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitialSpeed, InitialCrouchSpeed);
}

void UOFBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UOFBuffComponent::StartJumpBuff(float JumpZVelocity, float Duration)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
	MulticastJumpBuff(JumpZVelocity);
	GetWorld()->GetTimerManager().SetTimer(JumpBuffTimerHandle, this, &UOFBuffComponent::JumpBuffFinished, Duration, true);
}

void UOFBuffComponent::JumpBuffFinished()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpBuff(InitialJumpVelocity);
}

void UOFBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
}
