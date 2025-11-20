#pragma once

#include "CoreMinimal.h"
#include "Overfront/Public/PlayerState/OFPlayerState.h"
#include "FScoreboardEntry.generated.h"

USTRUCT(BlueprintType)
struct FScoreboardEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AOFPlayerState> Player = nullptr;

	UPROPERTY()
	float Score = 0.0f;

	FScoreboardEntry() : Player(nullptr), Score(0.f) {}

	FScoreboardEntry(AOFPlayerState* InPlayer, float InScore): Player(InPlayer), Score(InScore) {}
};  