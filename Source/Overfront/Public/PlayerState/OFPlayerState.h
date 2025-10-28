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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Replication Notifies **/
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
	void OnEliminated(float RespawnDelay, FString KillerName);
private:
	UPROPERTY()
	class AOverfrontCharacter* Character;
	UPROPERTY()
	class AOFPlayerController* PlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats = 0;
};
