// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OFProjectile.generated.h"

UCLASS(Abstract)
class OVERFRONT_API AOFProjectile : public AActor
{
	GENERATED_BODY()

public:
	AOFProjectile();
	virtual void Destroyed() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated)
	TSubclassOf<UDamageType> DamageType;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);

	UPROPERTY()
	FHitResult ProjectileHitResult;
	
	UPROPERTY(EditAnywhere, Category = "Projectile|Damage")
	float BaseDamage = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "Projectile|Properties")
	class UDA_ProjectileImpactsFX* ImpactData;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
};
