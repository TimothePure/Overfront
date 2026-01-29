// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ImpactResolver.generated.h"

class UDA_ProjectileImpactsFX;

USTRUCT(BlueprintType)
struct FImpactContext
{
	GENERATED_BODY()

	UPROPERTY()
	FHitResult Hit;

	// UPROPERTY()
	// AActor* InstigatorActor;
	//
	// UPROPERTY()
	// AController* InstigatorController;
	//
	// UPROPERTY()
	// TSubclassOf<UDamageType> DamageType;
	//
	// UPROPERTY()
	// float Damage = 0.f;

	UPROPERTY()
	bool bHitCharacter = false;
};

UCLASS()
class OVERFRONT_API UImpactResolver : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void ResolveImpactFX(UWorld* World, const FImpactContext& Context, UDA_ProjectileImpactsFX* ImpactData);
};
