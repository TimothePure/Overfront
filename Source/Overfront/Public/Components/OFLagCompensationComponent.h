// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OFLagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	FRotator Rotation;
	
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFRamePackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Time;
	
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfos;
};

USTRUCT()
struct FServerSideRewindResult
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool bHitConfirmed;
	
	UPROPERTY()
	FName HitBoxName = NAME_None;
};

class AOverfrontCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OVERFRONT_API UOFLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOFLagCompensationComponent();
	friend class AOverfrontCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFRamePackage& Package, const FColor& Color);
	
	/** Hitscan **/
	FServerSideRewindResult ServerSideRewind(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	
	/** Projectiles **/
	FServerSideRewindResult ProjectileServerSideRewind(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, 
		const FName BoneName, float HitTime, class AOFWeapon* DamageCauser);
	
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, 
		const FName BoneName, float HitTime, TSubclassOf<UDamageType> DamageType);
	
protected:
	virtual void BeginPlay() override;
	
	void SaveFramePackage();
	void SaveFramePackage(FFRamePackage& Package);
	
	FFRamePackage InterpolateBetweenFrames(const FFRamePackage& OlderFrame, const FFRamePackage& YoungerFrame, float HitTime);
	
	void CacheBoxPositions(AOverfrontCharacter* HitCharacter,  FFRamePackage& OutFramePackage);
	void MoveHitBoxes(AOverfrontCharacter* HitCharacter, const FFRamePackage& Package);
	void ResetHitBoxes(AOverfrontCharacter* HitCharacter, const FFRamePackage& Package);
	void EnableCharacterMeshCollision(AOverfrontCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	/** Hitscan **/
	FServerSideRewindResult ConfirmHit(const FFRamePackage& Package, AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	
	/** Projectile **/
	FServerSideRewindResult ProjectileConfirmHit(const FFRamePackage& Package,AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
	/** Shotgun **/ 
	FFRamePackage GetFrameToCheck(AOverfrontCharacter* HitCharacter, float HitTime);
	
private: 
	UPROPERTY()
	AOverfrontCharacter* Character;
	
	UPROPERTY()
	class AOFPlayerController* Controller;
	
	TDoubleLinkedList<FFRamePackage> FrameHistory;
	
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};
