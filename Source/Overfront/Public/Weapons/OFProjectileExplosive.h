// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFProjectile.h"
#include "OFProjectileExplosive.generated.h"

UCLASS(Abstract)
class OVERFRONT_API AOFProjectileExplosive : public AOFProjectile
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Explode();
	virtual void Destroyed() override;
	void SpawnTrailSystem();
	
	void StartExplodeTimer();
	void ExplodeTimerFinished();
	
	void StartDestroyTimer();
	void DestroyTimerFinished();

	UPROPERTY(EditAnywhere, Category = "Projectile|Properties")
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Projectile|Properties")
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true), Category = "Projectile|Damage")
	float DamageInnerRadius = 100.f;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true), Category = "Projectile|Damage")
	float DamageOuterRadius = 500.f;

private:
	FTimerHandle ExplodeTimerHandle;
	FTimerHandle DestroyTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Projectile|Timers")
	float ExplodeDuration = 3.f;
	
	UPROPERTY(EditAnywhere, Category = "Projectile|Timers")
	float DestroyDuration = 3.f;
};
