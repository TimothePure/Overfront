// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "OFFootstepAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFFootstepAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void PlaySoundFromSurfaceType(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float Volume = 1.f);

protected:
	UPROPERTY(EditAnywhere, Category="Trace")
	float TraceDistance = 300.f;

	UPROPERTY(EditAnywhere, Category="Sound")
	USoundBase* DefaultSound;

	UPROPERTY(EditAnywhere, Category="Sound")
	USoundBase* DirtSound;

	UPROPERTY(EditAnywhere, Category="Sound")
	USoundBase* ConcreteSound;

	UPROPERTY(EditAnywhere, Category="Sound")
	USoundBase* WoodSound;

	UPROPERTY(EditAnywhere, Category="Sound")
	USoundBase* SlushSound;
};
