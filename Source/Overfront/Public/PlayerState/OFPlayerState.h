// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OFPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API AOFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);
private:
	class AOverfrontCharacter* Character;
	class AOFPlayerController* PlayerController;
	
};
