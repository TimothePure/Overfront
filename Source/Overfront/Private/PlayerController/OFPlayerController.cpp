// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/OFPlayerController.h"

#include "Character/OverfrontCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "PlayerState/OFPlayerState.h"
#include "Widgets/OFCharacterOverlay.h"
#include "Widgets/OFDeathWidget.h"
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
        SetHUDHealth(OverfrontCharacter->GetHealth(), OverfrontCharacter->GetMaxHealth()); 
    }
}

void AOFPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    // Called inside BeginPlay and Controller->OnRep_PlayerState to ensure that the HUD is setup correctly
    if (AOFPlayerState* PS = GetPlayerState<AOFPlayerState>())
    {
        PS->AddToScore(0.f);
        PS->AddToDefeats(0);
    }
}

void AOFPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;

    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->HealthBar && HUD->CharacterOverlay->HealthText)
    {
        HUD->CharacterOverlay->SetHealth(Health, MaxHealth);
        const float HealthPercent = Health / MaxHealth;
        HUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
        FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        HUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
    }
}

void AOFPlayerController::SetHUDScore(float Score)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->ScoreAmount)
    {
        FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        HUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
    }
}

void AOFPlayerController::SetHUDDefeats(int32 Defeats)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->DefeatsAmount)
    {
        FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
        HUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
    }
}

void AOFPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->WeaponAmmoAmount)
    {
        FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
        HUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
    }
}

void AOFPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->CarriedAmmoAmount)
    {
        FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
        HUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
    }
}

void AOFPlayerController::SetHUDWeaponType(EWeaponType Type)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->WeaponType)
    {
        FString TypeText;
        const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EWeaponType"), true);
        if (!EnumPtr || Type == EWeaponType::EWT_MAX)
        {
            TypeText = FString("");
        } else
        {
            TypeText = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
        }
        
        HUD->CharacterOverlay->WeaponType->SetText(FText::FromString(TypeText));
    }
}

void AOFPlayerController::SetWeaponHUDVisibility(bool bVisible)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay)
    {
        HUD->CharacterOverlay->SetWeaponHUDVisibility(bVisible);
    }
}

void AOFPlayerController::OnEliminated(float RespawnDelay, FString KillerName)
{
    if (DeathWidgetClass && IsLocalController())
    {
        UOFDeathWidget* DeathWidget = CreateWidget<UOFDeathWidget>(this, DeathWidgetClass);
        if (DeathWidget)
        {
            DeathWidget->AddToViewport();
            DeathWidget->SetKillerNameText(KillerName);
            DeathWidget->StartRespawnTimer(RespawnDelay);
        }
    }
}

void AOFPlayerController::Client_OnEliminated_Implementation(float RespawnDelay, const FString& KillerName)
{
    OnEliminated(RespawnDelay, KillerName);
}
