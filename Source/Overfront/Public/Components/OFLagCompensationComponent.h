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
struct FFramePackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Time;
	
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfos;

	UPROPERTY()
	class AOverfrontCharacter* Character;
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

USTRUCT()
struct FShotgunPelletHit
{
	GENERATED_BODY()

	UPROPERTY()
	AOverfrontCharacter* HitCharacter = nullptr;

	UPROPERTY()
	FName BoneName = NAME_None;
};

USTRUCT()
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FShotgunPelletHit> PelletHits;
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
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
	
	/** Hitscan weapons **/
	FServerSideRewindResult HitscanServerSideRewind(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	UFUNCTION(Server, Reliable)
	void HitscanServerScoreRequest(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, 
		const FName BoneName, float HitTime, class AOFWeapon* DamageCauser);
	
	/** Projectiles weapons **/
	FServerSideRewindResult ProjectileServerSideRewind(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, 
		const FName BoneName, float HitTime, AOFWeapon* DamageCauser);

	/** Shotgun **/ 
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<AOverfrontCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime, AOFWeapon* DamageCauser);

	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<AOverfrontCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	
protected:
	virtual void BeginPlay() override;
	
	void SaveFramePackage();
	void SaveFramePackage(FFramePackage& Package);
	
	FFramePackage InterpolateBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	
	void CacheBoxPositions(AOverfrontCharacter* HitCharacter,  FFramePackage& OutFramePackage);
	void MoveHitBoxes(AOverfrontCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AOverfrontCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterHitboxesCollision(AOverfrontCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void EnableCharacterMeshCollision(AOverfrontCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	FFramePackage GetFrameToCheck(AOverfrontCharacter* HitCharacter, float HitTime);
	
	/** Hitscan **/
	FServerSideRewindResult HitscanConfirmHit(const FFramePackage& Package, AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	
	/** Projectile **/
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package,AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
	/** Shotgun **/
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

private: 
	UPROPERTY()
	AOverfrontCharacter* Character;
	
	UPROPERTY()
	class AOFPlayerController* Controller;
	
	TDoubleLinkedList<FFramePackage> FrameHistory;
	
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
};
