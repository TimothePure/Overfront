// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_ProjectileImpactsFX.generated.h"

class UFXSystemAsset;

USTRUCT(BlueprintType)
struct FProjectileImpactData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact")
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact")
	UFXSystemAsset* ImpactFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact")
	USoundBase* ImpactSound;
};

UCLASS()
class OVERFRONT_API UDA_ProjectileImpactsFX  : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FProjectileImpactData> SurfaceImpacts;
	
	UPROPERTY(EditAnywhere)
	UFXSystemAsset* CharacterImpactFX;

	UPROPERTY(EditAnywhere)
	USoundBase* CharacterImpactSound;
};
