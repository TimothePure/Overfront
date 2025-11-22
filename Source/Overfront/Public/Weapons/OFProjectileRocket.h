// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OFProjectile.h"
#include "OFProjectileRocket.generated.h"

UCLASS()
class OVERFRONT_API AOFProjectileRocket : public AOFProjectile
{
	GENERATED_BODY()

public:
	AOFProjectileRocket();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY(EditAnywhere)
	USoundBase* ProjectileLoop;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
	
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	float DamageInnerRadius = 100.f;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	float DamageOuterRadius = 500.f;

	FTimerHandle DestroyTimerHandle;

	UPROPERTY(EditAnywhere)
	float DestroyDuration = 3.f;

	void DestroyTimerFinished();
	
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;
};
