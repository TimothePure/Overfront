// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OFBuffComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OVERFRONT_API UOFBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOFBuffComponent();
	friend class AOverfrontCharacter;
	
	void Heal(float HealAmount, float HealingDuration, float TickRate = 0.1f);
	void StartSpeedBuff(float BaseSpeed, float CrouchSpeed, float Duration);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	
protected:
	virtual void BeginPlay() override;
	    
private:
	AOverfrontCharacter* Character;
	
	/** Health Buff **/
	
	FTimerHandle HealTimerHandle;
	float HealAmountTotal = 0.f;
	float HealDuration = 0.f;
	float HealTickRate = 0.1f;
	float HealAmountPerTick = 0.f;

	void ApplyHealingTick();
	void StopHealing();
	
	/** Speed Buff **/
	
	FTimerHandle SpeedBuffTimerHandle;
	float InitialSpeed;
	float InitialCrouchSpeed;
	
	void SpeedBuffFinished();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
};
