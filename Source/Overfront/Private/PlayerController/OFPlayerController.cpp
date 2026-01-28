// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/OFPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/OverfrontCharacter.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameModes/OverfrontGameMode.h"
#include "GameStates/OFGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/OFPlayerState.h"
#include "Weapons/OFWeapon.h"
#include "Widgets/OFAnnouncementWidget.h"
#include "Widgets/OFCharacterOverlay.h"
#include "Widgets/OFDeathWidget.h"
#include "Widgets/OFHUD.h"
#include "Widgets/OFScoreboardWidget.h"

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
    
    GetWorldTimerManager().SetTimer(CheckPingTimerHandle, this, &ThisClass::CheckPing, CheckPingFrequency, true);
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
    if (!OverfrontCharacter) return;
    SetHUDHealth(OverfrontCharacter->GetHealth(), OverfrontCharacter->GetMaxHealth()); 
    SetHUDShield(OverfrontCharacter->GetShield(), OverfrontCharacter->GetMaxShield());
    SetHUDGrenades(OverfrontCharacter->GetCombatComponent()->GetGrenades());
    
    if (OverfrontCharacter->GetEquippedWeapon())
    {
        SetWeaponHUDVisibility(true);
        SetHUDWeaponType(OverfrontCharacter->GetEquippedWeapon()->GetWeaponType());
        SetHUDWeaponAmmo(OverfrontCharacter->GetEquippedWeapon()->GetAmmo());
        SetHUDCarriedAmmo(OverfrontCharacter->GetCarriedAmmo());
    }
    else
    {
        SetWeaponHUDVisibility(false);
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

void AOFPlayerController::SetHUDShield(float Shield, float MaxShield)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;

    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->ShieldBar && HUD->CharacterOverlay->ShieldText)
    {
        HUD->CharacterOverlay->SetShield(Shield, MaxShield);
        // const float ShieldPercent = Shield / MaxShield;
        // HUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
        // FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
        // HUD->CharacterOverlay->ShieldText->SetText(FText::FromString(HealthText));
    } else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.Shield = Shield;
        PendingHUDData.MaxShield = MaxShield;
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
    } else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.WeaponAmmo = Ammo;
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
    else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.CarriedAmmo = Ammo;
    }
}

void AOFPlayerController::SetHUDWeaponType(EWeaponType Type)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->WeaponType)
    {
        FString TypeText;
        const UEnum* EnumPtr = StaticEnum<EWeaponType>();
        if (!EnumPtr || Type == EWeaponType::EWT_MAX)
        {
            TypeText = FString("");
        } else
        {
            TypeText = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Type)).ToString();
            TypeText.RemoveFromStart(TEXT("EWT_"));
        }
        
        HUD->CharacterOverlay->WeaponType->SetText(FText::FromString(TypeText));
    } else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.WeaponType = Type;
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
        if (CountdownTime < 0.f)
        {
            HUD->CharacterOverlay->MatchCountdownText->SetText(FText());
            HUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            return;
        }
        
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
        int32 Seconds = CountdownTime - Minutes * 60.f;
        FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

        if (CountdownTime < 10.f)
        {
            HUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
            HUD->CharacterOverlay->BlinkCountdown();
        }
        
        HUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
    }
}

void AOFPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->AnnouncementWidget && HUD->AnnouncementWidget->WarmupTime)
    {
        if (CountdownTime < 0.f)
        {
            HUD->AnnouncementWidget->WarmupTime->SetText(FText());
            return;
        }
        
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
        int32 Seconds = CountdownTime - Minutes * 60.f;
        FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        HUD->AnnouncementWidget->WarmupTime->SetText(FText::FromString(CountdownText));
    }
}

void AOFPlayerController::SetHUDScoreboard(const TArray<FScoreboardEntry>& Scoreboard)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;

    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->ScoreboardWidget)
    {
        HUD->CharacterOverlay->ScoreboardWidget->UpdateScoreboard(Scoreboard);
    }
}

void AOFPlayerController::SetHUDGrenades(int32 Grenades)
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;
    
    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->GrenadesAmount)
    {
        FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
        HUD->CharacterOverlay->GrenadesAmount->SetText(FText::FromString(GrenadesText));
    } else
    {
        PendingHUDData.bPendingData = true;
        PendingHUDData.Grenades = Grenades;
    }
}

void AOFPlayerController::SetHUDTime()
{
    float TimeLeft = 0.f;
    if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupDuration - GetServerTime();
    else if (MatchState == MatchState::InProgress) TimeLeft = WarmupDuration + MatchDuration - GetServerTime();
    else if (MatchState == MatchState::PostMatchCooldown) TimeLeft = WarmupDuration + MatchDuration + CooldownDuration - GetServerTime();

    uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
    
    if (CountdownInt != SecondsLeft)
    {   
        if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::PostMatchCooldown)
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
    SingleTripTime = RoundTripTime / 2;
    float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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
        ServerRequestPlayerTime(GetWorld()->GetTimeSeconds());
    }
}

void AOFPlayerController::InitHUDOverlay()
{
    if (PendingHUDData.bPendingData)
    {
        SetHUDHealth(PendingHUDData.Health, PendingHUDData.MaxHealth);
        SetHUDShield(PendingHUDData.Shield, PendingHUDData.MaxShield);
        SetHUDScore(PendingHUDData.Score);
        SetHUDDefeats(PendingHUDData.Defeats);
        SetHUDGrenades(PendingHUDData.Grenades);
        SetHUDCarriedAmmo(PendingHUDData.CarriedAmmo);
        
        if (PendingHUDData.CarriedAmmo >= 0)
        {
            SetHUDCarriedAmmo(PendingHUDData.CarriedAmmo);
        }

        if (PendingHUDData.WeaponAmmo >= 0)
        {
            SetHUDWeaponAmmo(PendingHUDData.WeaponAmmo);
        }

        if (PendingHUDData.WeaponType != EWeaponType::EWT_MAX)
        {
            SetHUDWeaponType(PendingHUDData.WeaponType);
        }
        
        PendingHUDData = FPendingHUDData();
    }
}

void AOFPlayerController::ServerCheckMatchState_Implementation()
{
    GameMode = GameMode == nullptr ? Cast<AOverfrontGameMode>(UGameplayStatics::GetGameMode(this)) : GameMode;
    if (GameMode)
    {
        MatchDuration = GameMode->MatchDuration;
        WarmupDuration = GameMode->WarmupDuration;
        CooldownDuration = GameMode->CooldownDuration;
        MatchState = GameMode->GetMatchState();
        ClientReceiveMatchState(MatchState, MatchDuration, WarmupDuration, CooldownDuration);
    }
}

void AOFPlayerController::ClientReceiveMatchState_Implementation(FName StateOfMatch, float Match, float Warmup, float Cooldown)
{
    MatchDuration = Match;
    WarmupDuration = Warmup;
    CooldownDuration = Cooldown;
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
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(CombatMappingContext);
        }
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
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(CombatMappingContext);
        }
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
        if (HUD->AnnouncementWidget && HUD->AnnouncementWidget->AnnouncementText && HUD->AnnouncementWidget->InfoText)
        {
            HUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
            FString AnnouncementText("New match starts in:");
            HUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnouncementText));

            FString InfoText;
            AOFGameState* GameState = Cast<AOFGameState>(UGameplayStatics::GetGameState(this));
            PlayerState = PlayerState == nullptr ? GetPlayerState<AOFPlayerState>() : PlayerState;
            if (GameState && PlayerState)
            {
                TArray<AOFPlayerState*>TopPlayers = GameState->GetTopScoringPlayers();
                if (TopPlayers.Num() <= 0)
                {
                    InfoText = FString("There is no winner");
                } else if (TopPlayers.Num() == 1)
                {
                    if (TopPlayers[0] == PlayerState)
                    {
                        InfoText = FString("You are the winner !");
                    } else
                    {
                        InfoText = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
                    }
                } else // More than one player
                {
                    InfoText = FString("Players tied for the win:\n");
                    for (auto TiedPlayer : TopPlayers)
                    {
                        InfoText.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
                    }
                }
            }
            HUD->AnnouncementWidget->InfoText->SetText(FText::FromString(InfoText));
        }
    }
}

void AOFPlayerController::StartHighPingWarning()
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;

    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->HighPingImage && HUD->CharacterOverlay->HighPingAnimation)
    {
        HUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
        HUD->CharacterOverlay->PlayAnimation(HUD->CharacterOverlay->HighPingAnimation, 0.f, 5); 
    }
}

void AOFPlayerController::StopHighPingWarning()
{
    HUD = HUD == nullptr ? Cast<AOFHUD>(GetHUD()) : HUD;

    if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->HighPingImage && HUD->CharacterOverlay->HighPingAnimation)
    {
        HUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
        if (HUD->CharacterOverlay->IsAnimationPlaying(HUD->CharacterOverlay->HighPingAnimation))
        {
            HUD->CharacterOverlay->StopAnimation(HUD->CharacterOverlay->HighPingAnimation);
        }
    }
}

void AOFPlayerController::CheckPing()
{
    PlayerState = PlayerState == nullptr ? GetPlayerState<AOFPlayerState>() : PlayerState;
    if (PlayerState->GetPingInMilliseconds() > HighPingThreshold)
    {
        StartHighPingWarning();
        GetWorldTimerManager().SetTimer(PingWarningTimerHandle, this, &ThisClass::StopHighPingWarning, HighPingWarningDuration, false);
    }
}