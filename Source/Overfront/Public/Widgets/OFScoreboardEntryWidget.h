// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OFScoreboardEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFScoreboardEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetEntry(const struct FScoreboardEntry& Entry);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;
};
