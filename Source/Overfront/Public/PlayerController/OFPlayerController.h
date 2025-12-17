// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Overfront/Enums/OFWeaponTypes.h"
#include "OFPlayerController.generated.h"

// Struct used to store the data the overlay needs to display when created
USTRUCT()
struct FPendingHUDData
{
	GENERATED_BODY()

	bool bPendingData = false;
	float Health;
	float MaxHealth;
	float Shield;
	float MaxShield;
	float Score = 0.f;
	int32 Defeats = 0;
	int32 Grenades = 0;
};

/**
 * 
 */
UCLASS()
class OVERFRONT_API AOFPlayerController : public APlayerController
{
	GENERATED_BODY()

	/** Default Input Mapping Context **/
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputMappingContext* DefaultMappingContext;

	/** Combat Input Mapping Context **/
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* CombatMappingContext;

public:
	virtual void SetupInputComponent() override;
	void OnEliminated(float RespawnDelay, FString KillerName);
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** HUD Updates Functions **/
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDWeaponType(EWeaponType Type);
	void SetWeaponHUDVisibility(bool bVisible);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDScoreboard(const TArray<struct FScoreboardEntry>& Scoreboard);
	void SetHUDGrenades(int32 Grenades);
	
	UFUNCTION(Client, Reliable)
	void Client_OnEliminated(float RespawnDelay, const FString& KillerName);

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	void SetHUDTime();
	
	/** Sync time between client and server **/

	// Requests the current server time, passing the current client time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestPlayerTime(float TimeOfClientRequest);
	
	// Report the current server time to the client in response to ServerRequestPlayerTime()
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time
	
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientReceiveMatchState(FName StateOfMatch, float Match, float Warmup, float Cooldown);

	void HandleMatchInProgress();
	void HandlePostMatchCooldown();
private:
	UPROPERTY()
	class AOFHUD* HUD;

	UPROPERTY()
	class AOverfrontGameMode* GameMode;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TSubclassOf<class UOFDeathWidget> DeathWidgetClass;

	float MatchDuration = 0.f;
	float WarmupDuration = 0.f;
	float CooldownDuration = 0.f;
	uint32 CountdownInt = 0;
	
	FTimerHandle TimeSyncTimerHandle;
	void TimerSyncUpdate();

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	/** Overlay display data initialization **/ 
	UPROPERTY()
	FPendingHUDData PendingHUDData;
	void InitHUDOverlay();
};
