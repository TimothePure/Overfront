// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/OFPlayerState.h"

#include "Character/OverfrontCharacter.h"
#include "PlayerController/OFPlayerController.h"

void AOFPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AOverfrontCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AOFPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}

void AOFPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = Character == nullptr ? Cast<AOverfrontCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AOFPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}
