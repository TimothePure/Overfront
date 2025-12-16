// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OFBuffComponent.h"

#include "Character/OverfrontCharacter.h"


UOFBuffComponent::UOFBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOFBuffComponent::Heal(float HealAmount, float HealingDuration, float TickRate)
{
	StopHealing();
	
	HealAmountTotal = HealAmount;
	HealDuration    = HealingDuration;
	HealTickRate    = TickRate;

	const float TickCount = HealingDuration / TickRate;

	HealAmountPerTick = HealAmount / TickCount;

	// Start timer
	GetWorld()->GetTimerManager().SetTimer(
		HealTimerHandle,
		this,
		&UOFBuffComponent::ApplyHealingTick,
		TickRate,
		true
	);
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

void UOFBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}
