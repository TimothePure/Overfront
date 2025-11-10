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
	AOverfrontGameMode();
	virtual void PlayerEliminated(class AOverfrontCharacter* EliminatedCharacter, class AOFPlayerController* VictimController, AOFPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
	
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	AActor* GetFurthestPlayerStart(AController* EliminatedController) const;

private:
	UPROPERTY(EditAnywhere)
	float RespawnDelay = 5.f;

	FTimerHandle WarmupTimerHandle;
	void WarmupTimerFinished();
};
