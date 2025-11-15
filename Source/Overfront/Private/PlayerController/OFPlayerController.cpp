// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/OFPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/OverfrontCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameModes/OverfrontGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/OFPlayerState.h"
#include "Widgets/OFAnnouncementWidget.h"
#include "Widgets/OFCharacterOverlay.h"
#include "Widgets/OFDeathWidget.h"
#include "Widgets/OFHUD.h"

class UEnhancedInputLocalPlayerSubsystem;

void AOFPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    HUD = Cast<AOFHUD>(GetHUD());
    ServerCheckMatchState();

    UE_LOG(LogTemp, Warning, TEXT("AOFPlayerController::BeginPlay() - IsLocalController=%d, LocalPlayer=%s, NetMode=%d"),
    IsLocalController(),
    GetLocalPlayer() ? *GetLocalPlayer()->GetName() : TEXT("NONE"),
    (int32)GetNetMode());

}

void AOFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOFPlayerController, MatchState);
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

void AOFPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    SetHUDTime();
}

void AOFPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (LocalPlayer)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->ClearAllMappings();
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
            Subsystem->AddMappingContext(CombatMappingContext, 0);
        }
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
    } else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.Health = Health;
        PendingHUDData.MaxHealth = MaxHealth;
    }
}

void AOFPlayerController::SetHUDScore(float Score)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->ScoreAmount)
    {
        FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        HUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
    } else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.Score = Score;
    }
}

void AOFPlayerController::SetHUDDefeats(int32 Defeats)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->DefeatsAmount)
    {
        FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
        HUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
    } else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.Defeats = Defeats;
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

void AOFPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->MatchCountdownText)
    {
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
        int32 Seconds = CountdownTime - Minutes * 60.f;
        FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        HUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
    }
}

void AOFPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->AnnouncementWidget && HUD->AnnouncementWidget->WarmupTime)
    {
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
        int32 Seconds = CountdownTime - Minutes * 60.f;
        FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        HUD->AnnouncementWidget->WarmupTime->SetText(FText::FromString(CountdownText));
    }
}

void AOFPlayerController::SetHUDTime()
{
    float TimeLeft = 0.f;
    if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupDuration - GetServerTime();
    else if (MatchState == MatchState::InProgress) TimeLeft = WarmupDuration + MatchDuration - GetServerTime();
    
    uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
    if (CountdownInt != SecondsLeft)
    {   
        if (MatchState == MatchState::WaitingToStart)
        {
            SetHUDAnnouncementCountdown(TimeLeft);
        }

        if (MatchState == MatchState::InProgress)
        {
            SetHUDMatchCountdown(TimeLeft);
        }
    }

    CountdownInt = SecondsLeft;
}

void AOFPlayerController::ServerRequestPlayerTime_Implementation(float TimeOfClientRequest)
{
    float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
    ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AOFPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
    float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime / 2);
    ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AOFPlayerController::GetServerTime()
{
    return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AOFPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();
    if (IsLocalController())
    {
        ServerRequestPlayerTime(GetWorld()->GetTimeSeconds());
        GetWorld()->GetTimerManager().SetTimer(TimeSyncTimerHandle, this, &AOFPlayerController::TimerSyncUpdate, TimeSyncFrequency, true);
    }
}

void AOFPlayerController::TimerSyncUpdate()
{
    if (IsLocalController())
    {
        UE_LOG(LogTemp, Display, TEXT("AOFPlayerController::TimerSyncUpdate"));
        ServerRequestPlayerTime(GetWorld()->GetTimeSeconds());
        UE_LOG(LogTemp, Display, TEXT("%f"), ClientServerDelta);
    }
}

void AOFPlayerController::InitHUDOverlay()
{
    if (PendingHUDData.bPendingData)
    {
        SetHUDHealth(PendingHUDData.Health, PendingHUDData.MaxHealth);
        SetHUDScore(PendingHUDData.Score);
        SetHUDDefeats(PendingHUDData.Defeats);
        PendingHUDData = FPendingHUDData();
    }
}


void AOFPlayerController::ServerCheckMatchState_Implementation()
{
    if (AOverfrontGameMode* GameMode = Cast<AOverfrontGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        MatchDuration = GameMode->MatchDuration;
        WarmupDuration = GameMode->WarmupDuration;
        MatchState = GameMode->GetMatchState();
        ClientReceiveMatchState(MatchState, MatchDuration, WarmupDuration);
    }
}

void AOFPlayerController::ClientReceiveMatchState_Implementation(FName StateOfMatch, float Match, float Warmup)
{
    MatchDuration = Match;
    WarmupDuration = Warmup;
    MatchState = StateOfMatch;
    OnMatchStateSet(MatchState);
}

void AOFPlayerController::OnMatchStateSet(FName State)
{
    MatchState = State;
    
    if (!IsLocalController()) return;
    
    if (MatchState == MatchState::WaitingToStart && HUD)
    {
        HUD->AddAnnouncementWidget();
    }
    else if (MatchState == MatchState::PostMatchCooldown)
    {
        HandlePostMatchCooldown();
    }
    else if (MatchState == MatchState::InProgress)
    {
        HandleMatchInProgress();
    }
}

void AOFPlayerController::OnRep_MatchState()
{
    if (!IsLocalController()) return;
    
    if (MatchState == MatchState::WaitingToStart && HUD)
    {
        HUD->AddAnnouncementWidget();
    }
    else if (MatchState == MatchState::PostMatchCooldown)
    {
        HandlePostMatchCooldown();
    }
    else if (MatchState == MatchState::InProgress)
    {
        HandleMatchInProgress();
    }
}

void AOFPlayerController::HandleMatchInProgress()
{
    if (HUD)
    {
        HUD->AddCharacterOverlay();
        InitHUDOverlay();
        if (HUD->AnnouncementWidget)
        {
            HUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void AOFPlayerController::HandlePostMatchCooldown()
{
    if (HUD)
    {
        HUD->CharacterOverlay->RemoveFromParent();
        if (HUD->AnnouncementWidget)
        {
            HUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
        }
    }
}