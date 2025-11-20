// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "OFGameState.generated.h"

class AOFPlayerState;

/**
 * 
 */
UCLASS()
class OVERFRONT_API AOFGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void UpdateScoreboard(AOFPlayerState* ScoringPlayer);

	void RebuildScoreboardFromPlayers();

	TArray<AOFPlayerState*> GetTopScoringPlayers() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Scoreboard)
	TArray<struct FScoreboardEntry> Scoreboard;
	
	UFUNCTION()
	void OnRep_Scoreboard();
};
