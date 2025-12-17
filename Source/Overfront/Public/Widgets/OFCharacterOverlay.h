// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OFCharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Health Bar **/
	
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;
	
	void SetHealth(float NewHealth, float NewMaxHealth);
	
	/** Shield Bar **/
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;
	
	void SetShield(float NewShield, float NewMaxShield);
	
	/** Score **/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;
	
	UPROPERTY(meta=(BindWidget))
	class UOFScoreboardWidget* ScoreboardWidget;

	UPROPERTY(meta = (BindWidget))
	class UImage* BloodOverlayImage;
	
	/** Weapons **/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* WeaponPanel;

	void SetWeaponHUDVisibility(bool bVisible);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponType;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesAmount;
	
	/** Countdown **/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;
	
	void BlinkCountdown();

private:
	float Health;
	float MaxHealth;
	
	float Shield;
	float MaxShield;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	float MaxBloodOpacity = 0.3f;

	FTimerHandle FadeTimerHandle;
	FTimerHandle FadeStopHandle;

	bool bFadingOut = true;
	float Elapsed = 0.f;
	float HalfInterval = 0.f;
	
	void UpdateSmoothFade();
};
