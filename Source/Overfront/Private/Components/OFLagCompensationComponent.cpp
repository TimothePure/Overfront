// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OFLagCompensationComponent.h"

UOFLagCompensationComponent::UOFLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOFLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOFLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

