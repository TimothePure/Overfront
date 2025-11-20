// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/OFGameState.h"

#include "Net/UnrealNetwork.h"
#include "Overfront/Struct/FScoreboardEntry.h"
#include "PlayerController/OFPlayerController.h"
#include "PlayerState/OFPlayerState.h"

void AOFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(AOFGameState, TopScoringPlayers);
	DOREPLIFETIME(AOFGameState, Scoreboard);
}

void AOFGameState::UpdateScoreboard(AOFPlayerState* ScoringPlayer)
{
	if (!ScoringPlayer) return;

	// Find existing entry
	for (FScoreboardEntry& Entry : Scoreboard)
	{
		if (Entry.Player == ScoringPlayer)
		{
			Entry.Score = ScoringPlayer->GetScore();
            
			Scoreboard.Sort([](const FScoreboardEntry& A, const FScoreboardEntry& B)
			{
				return A.Score > B.Score;
			});

			OnRep_Scoreboard();
			return;
		}
	}

	// Create new entry if not found
	FScoreboardEntry NewEntry(ScoringPlayer, ScoringPlayer->GetScore());
	Scoreboard.Add(NewEntry);

	Scoreboard.Sort([](const FScoreboardEntry& A, const FScoreboardEntry& B)
	{
		return A.Score > B.Score;
	});

	OnRep_Scoreboard();
}

void AOFGameState::OnRep_Scoreboard()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AOFPlayerController* PC = Cast<AOFPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			PC->SetHUDScoreboard(Scoreboard);
		}
	}
}

void AOFGameState::RebuildScoreboardFromPlayers()
{
	Scoreboard.Empty();

	for (APlayerState* PS : PlayerArray)
	{
		if (AOFPlayerState* OFPS = Cast<AOFPlayerState>(PS))
		{
			UpdateScoreboard(OFPS);
		}
	}
}

TArray<AOFPlayerState*> AOFGameState::GetTopScoringPlayers() const
{
	TArray<AOFPlayerState*> TopPlayers;

	if (Scoreboard.Num() == 0) return TopPlayers;

	float TopScore = Scoreboard[0].Score;

	for (const FScoreboardEntry& Entry : Scoreboard)
	{
		if (!Entry.Player.IsValid()) continue;

		if (Entry.Score == TopScore)
		{
			TopPlayers.Add(Entry.Player.Get());
		}
		else
		{
			break; 
		}
	}

	return TopPlayers;
}