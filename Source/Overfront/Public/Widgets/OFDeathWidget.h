// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OFDeathWidget.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFDeathWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetKillerNameText(FString KillerName);
	void StartRespawnTimer(float RespawnDelay);
	void UpdateRespawnTimer();
private:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* DeathMessage;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* KillerNameText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* CountdownText;

	float TimerSecondsRemaining;
	FTimerHandle RespawnTimerHandle;
};
