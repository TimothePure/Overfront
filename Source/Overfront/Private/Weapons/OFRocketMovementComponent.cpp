// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFRocketMovementComponent.h"


UOFRocketMovementComponent::UOFRocketMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

UOFRocketMovementComponent::EHandleBlockingHitResult UOFRocketMovementComponent::HandleBlockingHit(
	const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void UOFRocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Rockets should not stop, only explodes when their collision box detects a hit
}