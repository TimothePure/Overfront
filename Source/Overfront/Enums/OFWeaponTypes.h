#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_SubmachineGun UMETA(DisplayName = "SMG"),

	EWT_MAX UMETA(DisplayName = "Default Max"),
};
