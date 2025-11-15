// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/OverfrontGameMode.h"
#include "Character/OverfrontCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/OFPlayerController.h"
#include "PlayerState/OFPlayerState.h"

namespace MatchState
{
	const FName PostMatchCooldown = FName("PostMatchCooldown");
}

AOverfrontGameMode::AOverfrontGameMode()
{
	bDelayedStart = true;
}

void AOverfrontGameMode::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(WarmupTimerHandle, this, &AOverfrontGameMode::WarmupTimerFinished, WarmupDuration, false);
}

void AOverfrontGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AOFPlayerController* OFPlayerController = Cast<AOFPlayerController>(It->Get()))
		{
			OFPlayerController->OnMatchStateSet(MatchState);	
		}
	}
	if (MatchState == MatchState::InProgress)
	{
		GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AOverfrontGameMode::MatchTimerFinished, MatchDuration, false);
	}
}

void AOverfrontGameMode::PlayerEliminated(AOverfrontCharacter* EliminatedCharacter, AOFPlayerController* VictimController, AOFPlayerController* AttackerController)
{
	AOFPlayerState* AttackerPlayerState = AttackerController ? Cast<AOFPlayerState>(AttackerController->PlayerState) : nullptr;
	AOFPlayerState* VictimPlayerState = VictimController ? Cast<AOFPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
		if (AttackerPlayerState)
		{
			VictimPlayerState->OnEliminated(RespawnDelay, AttackerPlayerState->GetPlayerName());
		}
	}
	
	if (EliminatedCharacter) 
	{
		EliminatedCharacter->OnEliminated(RespawnDelay);
	}
}

void AOverfrontGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if (EliminatedController)
	{
		if (AActor* BestPlayerStart = GetFurthestPlayerStart(EliminatedController))
		{
			RestartPlayerAtPlayerStart(EliminatedController, BestPlayerStart);
		}
		else
		{
			Super::RestartPlayer(EliminatedController);
		}
	}
}

AActor* AOverfrontGameMode::GetFurthestPlayerStart(AController* EliminatedController) const
{
	if (!EliminatedController) return nullptr;

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.Num() == 0) return nullptr;

	TArray<AActor*> LivingPlayers;
	UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), LivingPlayers);

	AActor* BestStart = nullptr;
	float BestMinDistance = -1.f;

	for (AActor* Start : PlayerStarts)
	{
		if (!Start) continue;

		float ClosestPlayerDistance = TNumericLimits<float>::Max();

		for (AActor* Player : LivingPlayers)
		{
			if (!Player || Player == EliminatedController->GetPawn()) continue;

			float Dist = FVector::Dist(Player->GetActorLocation(), Start->GetActorLocation());
			ClosestPlayerDistance = FMath::Min(ClosestPlayerDistance, Dist);
		}

		if (ClosestPlayerDistance > BestMinDistance)
		{
			BestMinDistance = ClosestPlayerDistance;
			BestStart = Start;
		}
	}

	return BestStart;
}

void AOverfrontGameMode::WarmupTimerFinished()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		StartMatch();
	}
}

void AOverfrontGameMode::MatchTimerFinished()
{
	if (MatchState == MatchState::InProgress)
	{
		SetMatchState(MatchState::PostMatchCooldown);
	}
}
