// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OFScoreboardWidget.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateScoreboard(const TArray<struct FScoreboardEntry>& Scoreboard);

protected:
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ScoreboardList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scoreboard")
	TSubclassOf<class UOFScoreboardEntryWidget> EntryWidgetClass;

	UPROPERTY(EditAnywhere, Category="Scoreboard")
	int MaxVisiblePlayers = 5;
};
