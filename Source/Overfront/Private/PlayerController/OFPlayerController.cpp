// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/OFPlayerController.h"

#include "Character/OverfrontCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Widgets/OFCharacterOverlay.h"
#include "Widgets/OFHUD.h"

void AOFPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    HUD = Cast<AOFHUD>(GetHUD());
}

void AOFPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AOverfrontCharacter* OverfrontCharacter = Cast<AOverfrontCharacter>(InPawn);
    if (OverfrontCharacter)
    {
        OverfrontCharacter->UpdateHUDHealth();
    }
}

void AOFPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->HealthBar && HUD->CharacterOverlay->HealthText)
    {
        const float HealthPercent = Health / MaxHealth;
        HUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
        FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        HUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
    }
}
