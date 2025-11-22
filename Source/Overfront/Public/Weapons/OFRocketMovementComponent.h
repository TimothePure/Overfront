// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "OFRocketMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OVERFRONT_API UOFRocketMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	UOFRocketMovementComponent();

protected:
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0, const FVector& MoveDelta = FVector::ZeroVector) override;
};
