// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/OverfrontGameMode.h"
#include "Character/OverfrontCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void AOverfrontGameMode::PlayerEliminated(AOverfrontCharacter* EliminatedCharacter, AOFPlayerController* VictimController, AOFPlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->OnEliminated();
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
