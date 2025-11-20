// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OFScoreboardWidget.h"

#include "VisualizeTexture.h"
#include "Components/VerticalBox.h"
#include "Widgets/OFScoreboardEntryWidget.h"
#include "Overfront/Struct/FScoreboardEntry.h"

void UOFScoreboardWidget::UpdateScoreboard(const TArray<FScoreboardEntry>& Scoreboard)
{
	if (!ScoreboardList || !EntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScoreboardList or EntryWidgetClass not set!"));
		return;
	}

	// Clear old entries
	ScoreboardList->ClearChildren();

	TArray<FScoreboardEntry> SortedScoreboard = Scoreboard;
	SortedScoreboard.Sort([](const FScoreboardEntry& A, const FScoreboardEntry& B) {
		return A.Score > B.Score;
	});

	// Add new entries
	for (const FScoreboardEntry& Entry : SortedScoreboard)
	{
		UOFScoreboardEntryWidget* EntryWidget =
			CreateWidget<UOFScoreboardEntryWidget>(GetWorld(), EntryWidgetClass);

		if (!EntryWidget)
			continue;

		EntryWidget->SetEntry(Entry);
		ScoreboardList->AddChild(EntryWidget);
	}
}

