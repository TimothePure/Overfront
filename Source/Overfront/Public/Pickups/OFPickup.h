// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OFPickup.generated.h"

UCLASS()
class OVERFRONT_API AOFPickup : public AActor
{
	GENERATED_BODY()

public:
	AOFPickup();
	virtual void Tick(float DeltaSeconds) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;
	
private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;
	
	UPROPERTY(EditAnywhere)
	USoundBase* PickupSound;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;
	
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComp;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;
};
