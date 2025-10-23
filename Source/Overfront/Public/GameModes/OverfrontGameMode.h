// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OverfrontGameMode.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API AOverfrontGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AOverfrontCharacter* EliminatedCharacter, class AOFPlayerController* VictimController, AOFPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);

protected:
	AActor* GetFurthestPlayerStart(AController* EliminatedController) const;
};
