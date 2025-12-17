// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OFCharacterOverlay.h"

#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOFCharacterOverlay::SetHealth(float NewHealth, float NewMaxHealth)
{
	Health = NewHealth;
	MaxHealth = NewMaxHealth;
	const float HealthPercent = Health / MaxHealth;

	// Health Bar
	HealthBar->SetPercent(HealthPercent);

	// Health Text
	FString Text = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
	HealthText->SetText(FText::FromString(Text));

	// Blood Overlay
	BloodOverlayImage->SetOpacity(MaxBloodOpacity * (1 - HealthPercent));
}

void UOFCharacterOverlay::SetShield(float NewShield, float NewMaxShield)
{
	Shield = NewShield;
	MaxShield = NewMaxShield;
	const float ShieldPercent = Shield / MaxShield;

	// Health Bar
	ShieldBar->SetPercent(ShieldPercent);

	// Health Text
	FString Text = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
	ShieldText->SetText(FText::FromString(Text));
}

void UOFCharacterOverlay::SetWeaponHUDVisibility(bool bVisible)
{
	bVisible ? WeaponPanel->SetVisibility(ESlateVisibility::Visible) : WeaponPanel->SetVisibility(ESlateVisibility::Hidden);
}

void UOFCharacterOverlay::BlinkCountdown()
{
	if (!MatchCountdownText) return;

	bFadingOut = true;
	MatchCountdownText->SetRenderOpacity(1.f);
	Elapsed = 0.f;

	// Start fade timer (smooth)
	if (GetWorld()->GetTimerManager().IsTimerActive(FadeTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &UOFCharacterOverlay::UpdateSmoothFade, 0.05f, true);
}

void UOFCharacterOverlay::UpdateSmoothFade()
{
	if (!MatchCountdownText) return;
	
	Elapsed += 0.05f;
	float Alpha = FMath::Clamp(Elapsed / 0.6f, 0.f, 1.f);

	float NewOpacity = bFadingOut ? FMath::Lerp(1.0f, 0.0f, Alpha) : FMath::Lerp(0.0f, 1.0f, Alpha);
	MatchCountdownText->SetRenderOpacity(NewOpacity);
	
	if (Alpha >= 1.f)
	{
		Elapsed = 0.f;
		bFadingOut = !bFadingOut;
	}
}
