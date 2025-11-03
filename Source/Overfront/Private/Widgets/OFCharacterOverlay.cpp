// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OFCharacterOverlay.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOFCharacterOverlay::SetHealth(float NewHealth, float NewMaxHealth)
{
	this->Health = NewHealth;
	this->MaxHealth = NewMaxHealth;
	const float HealthPercent = Health / MaxHealth;

	// Health Bar
	HealthBar->SetPercent(HealthPercent);

	// Health Text
	FString Text = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
	HealthText->SetText(FText::FromString(Text));

	// Blood Overlay
	BloodOverlayImage->SetOpacity(MaxBloodOpacity * (1 - HealthPercent));
}

void UOFCharacterOverlay::SetAmmoHUDVisibility(bool bVisible)
{
	bVisible ? AmmoBox->SetVisibility(ESlateVisibility::Visible) : AmmoBox->SetVisibility(ESlateVisibility::Hidden);
}
