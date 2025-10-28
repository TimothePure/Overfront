// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OFDeathWidget.h"

#include "Components/TextBlock.h"

void UOFDeathWidget::SetKillerNameText(FString KillerName)
{
	KillerNameText->SetText(FText::FromString(KillerName));
}

void UOFDeathWidget::StartRespawnTimer(float RespawnDelay)
{
	TimerSecondsRemaining = RespawnDelay;
	CountdownText->SetText(FText::FromString(FString::FromInt(TimerSecondsRemaining)));
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::UpdateRespawnTimer,1.0f,true);
}

void UOFDeathWidget::UpdateRespawnTimer()
{
	TimerSecondsRemaining--;
	if (TimerSecondsRemaining > 0)
	{
		CountdownText->SetText(FText::FromString(FString::FromInt(TimerSecondsRemaining)));
	} else
	{
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
		RemoveFromParent();
	}
}
