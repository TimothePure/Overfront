// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OFBulletShell.generated.h"

UCLASS()
class OVERFRONT_API AOFBulletShell : public AActor
{
	GENERATED_BODY()

public:
	AOFBulletShell();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);


private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShellMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	USoundBase* ShellSound;
};
