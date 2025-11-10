// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Overfront/Enums/OFWeaponTypes.h"
#include "OFPlayerController.generated.h"

USTRUCT()
struct FPendingHUDData
{
	GENERATED_BODY()

	bool bPendingData = false;
	float Health;
	float MaxHealth;
	float Score = 0.f;
	int32 Defeats = 0;
};

/**
 * 
 */
UCLASS()
class OVERFRONT_API AOFPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDWeaponType(EWeaponType Type);
	void SetWeaponHUDVisibility(bool bVisible);
	void SetHUDMatchCountdown(float CountdownTime);
	void OnEliminated(float RespawnDelay, FString KillerName);
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
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

private:
	class AOFHUD* HUD; 

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TSubclassOf<class UOFDeathWidget> DeathWidgetClass;

	float MatchTime = 120.f;

	uint32 CountdownInt = 0;
	
	FTimerHandle TimeSyncTimerHandle;
	void TimerSyncUpdate();

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	FPendingHUDData PendingHUDData;
	void InitHUDOverlay();
};
