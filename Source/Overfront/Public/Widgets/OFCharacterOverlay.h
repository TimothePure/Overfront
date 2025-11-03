// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "OFCharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	class UImage* BloodOverlayImage;
	
	void SetHealth(float NewHealth, float NewMaxHealth);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* AmmoBox;

	void SetAmmoHUDVisibility(bool bVisible);
	
private:
	float Health;
	float MaxHealth;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	float MaxBloodOpacity = 0.3f;
};
