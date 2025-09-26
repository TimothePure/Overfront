// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/OBLobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void AOBLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int NumberPlayers = GameState.Get()->PlayerArray.Num();
	if (NumberPlayers == 2)
	{
		if (UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString(TEXT("/Game/Levels/LVL_Game?listen")));
		}
	}
}

void AOBLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
