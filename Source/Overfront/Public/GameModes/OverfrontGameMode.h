// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OverfrontGameMode.generated.h"


namespace MatchState
{
	extern OVERFRONT_API const FName PostMatchCooldown; // Match duration has been reached. Display winner and begin cooldown.
}
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
	float WarmupDuration = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchDuration = 300.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownDuration = 10.f;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	AActor* GetFurthestPlayerStart(AController* EliminatedController) const;

private:
	UPROPERTY(EditAnywhere)
	float RespawnDelay = 5.f;

	FTimerHandle WarmupTimerHandle;
	void WarmupTimerFinished();
	
	FTimerHandle MatchTimerHandle;
	void MatchTimerFinished();

	FTimerHandle CooldownTimerHandle;
	void CooldownTimerFinished();
};
