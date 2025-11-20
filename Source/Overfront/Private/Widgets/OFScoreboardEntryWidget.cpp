// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/OFScoreboardEntryWidget.h"

#include "Overfront/Struct/FScoreboardEntry.h"
#include "Components/TextBlock.h"
#include "PlayerState/OFPlayerState.h"



void UOFScoreboardEntryWidget::SetEntry(const FScoreboardEntry& Entry)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(Entry.Player.Get() ? Entry.Player->GetPlayerName() : TEXT("Unknown")));
	}

	if (ScoreText)
	{
		ScoreText->SetText(FText::AsNumber(FMath::FloorToInt(Entry.Score)));
	}
}
