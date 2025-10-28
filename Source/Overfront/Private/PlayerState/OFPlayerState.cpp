// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/OFPlayerState.h"
#include "Character/OverfrontCharacter.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/OFPlayerController.h"

void AOFPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOFPlayerState, Defeats);
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

void AOFPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	Character = Character == nullptr ? Cast<AOverfrontCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AOFPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDefeats(Defeats);
		}
	}
}

void AOFPlayerState::OnEliminated(float RespawnDelay, FString KillerName)
{
	Character = Character == nullptr ? Cast<AOverfrontCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AOFPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->Client_OnEliminated(RespawnDelay, KillerName);
		}
	}
}

void AOFPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AOverfrontCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AOFPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDefeats(Defeats);
		}
	}
}
