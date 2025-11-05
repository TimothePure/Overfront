// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Overfront/Enums/OFWeaponTypes.h"
#include "OFPlayerController.generated.h"

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
	void OnEliminated(float RespawnDelay, FString KillerName);
	UFUNCTION(Client, Reliable)
	void Client_OnEliminated(float RespawnDelay, const FString& KillerName);
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	
private:
	class AOFHUD* HUD; 

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TSubclassOf<class UOFDeathWidget> DeathWidgetClass;
};
