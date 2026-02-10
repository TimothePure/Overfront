// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OFLagCompensationComponent.h"

#include "Character/OverfrontCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Overfront/Overfront.h"
#include "Weapons/OFWeapon.h"
#include "Weapons/Damage/OFWeaponDamageType.h"

UOFLagCompensationComponent::UOFLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOFLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	FFramePackage Package;
	SaveFramePackage(Package);
}

void UOFLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFramePackage();
}

void UOFLagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfos)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Color, false, 4.f);
	}
}
void UOFLagCompensationComponent::HitscanServerScoreRequest_Implementation(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, const FName BoneName, float HitTime,  AOFWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = HitscanServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	
	if (Character && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{
		if (UOFWeaponDamageType* WeaponDamage = DamageCauser->DamageType->GetDefaultObject<UOFWeaponDamageType>())
		{
			UGameplayStatics::ApplyDamage(HitCharacter, WeaponDamage->DetermineDamageAmount(Confirm.HitBoxName), Character->Controller, DamageCauser, DamageCauser->DamageType);
		}
	}
}

FServerSideRewindResult UOFLagCompensationComponent::HitscanServerSideRewind(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	
	return HitscanConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void UOFLagCompensationComponent::ProjectileServerScoreRequest_Implementation(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize100& InitialVelocity, const FName BoneName, float HitTime, AOFWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	
	if (Character && HitCharacter && Confirm.bHitConfirmed)
	{
		if (UOFWeaponDamageType* WeaponDamage = DamageCauser->DamageType->GetDefaultObject<UOFWeaponDamageType>())
		{
			UGameplayStatics::ApplyDamage(HitCharacter, WeaponDamage->DetermineDamageAmount(Confirm.HitBoxName), Character->Controller, DamageCauser, DamageCauser->DamageType);
		}
	}
}

FServerSideRewindResult UOFLagCompensationComponent::ProjectileServerSideRewind(AOverfrontCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

void UOFLagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<AOverfrontCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AOFWeapon* DamageCauser)
{
	if (!Character || !DamageCauser) return;
	
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	UOFWeaponDamageType* WeaponDamage = DamageCauser->DamageType ? DamageCauser->DamageType->GetDefaultObject<UOFWeaponDamageType>() : nullptr;

	if (!WeaponDamage) return;

	TMap<AOverfrontCharacter*, float> DamageMap;
	
	for (const FShotgunPelletHit& Pellet : Confirm.PelletHits)
	{
		const float PelletDamage = WeaponDamage->DetermineDamageAmount(Pellet.BoneName);

		DamageMap.FindOrAdd(Pellet.HitCharacter) += PelletDamage;
	}

	for (const auto& Damage : DamageMap)
	{
		UGameplayStatics::ApplyDamage(Damage.Key, Damage.Value, Character->Controller, DamageCauser, DamageCauser->DamageType);
	}
}

FShotgunServerSideRewindResult UOFLagCompensationComponent::ShotgunServerSideRewind(const TArray<AOverfrontCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;

	for (AOverfrontCharacter* HitCharacter : HitCharacters)
	{
		FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
		FramesToCheck.Add(FrameToCheck);
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FFramePackage UOFLagCompensationComponent::GetFrameToCheck(AOverfrontCharacter* HitCharacter, float HitTime)
{
	if (HitCharacter == nullptr || HitCharacter->GetLagCompensationComponent() == nullptr || 
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr || 
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr) return FFramePackage();
	
	const TDoubleLinkedList<FFramePackage>& HitCharacterHistory = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = HitCharacterHistory.GetTail()->GetValue().Time;
	const float NewestHistoryTime = HitCharacterHistory.GetHead()->GetValue().Time;
	
	if (OldestHistoryTime > HitTime) return FFramePackage(); // Too far back - too laggy for SSR
	if (OldestHistoryTime == HitTime )
	{
		return HitCharacterHistory.GetHead()->GetValue();
	}
	if (NewestHistoryTime <= HitTime) 
	{
		return HitCharacterHistory.GetHead()->GetValue();
	}
	
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = HitCharacterHistory.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	
	while (Older->GetValue().Time > HitTime)
	{
		// March back until: Older.Time < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr) break;
		
		Older = Older->GetNextNode();
		
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	
	if (Older->GetValue().Time == HitTime) // Highly unlikely but we found the frame to check
	{
		return Older->GetValue();
	}
	
	// Interpolate between Older and Younger
	return InterpolateBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
}

FFramePackage UOFLagCompensationComponent::InterpolateBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.0f, 1.0f);
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	InterpFramePackage.Character = OlderFrame.Character;
	
	for (auto& YoungerPair : YoungerFrame.HitBoxInfos)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBoxInfo = OlderFrame.HitBoxInfos[BoxInfoName];
		const FBoxInformation& YoungerBoxInfo = YoungerPair.Value;
		
		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBoxInfo.Location, YoungerBoxInfo.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBoxInfo.Rotation, YoungerBoxInfo.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBoxInfo.BoxExtent;
		
		InterpFramePackage.HitBoxInfos.Add(BoxInfoName, InterpBoxInfo);
	}
	
	return InterpFramePackage;
}

FServerSideRewindResult UOFLagCompensationComponent::HitscanConfirmHit(const FFramePackage& Package, AOverfrontCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr || Package.HitBoxInfos.Num() == 0) return FServerSideRewindResult();
	
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveHitBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; 
	if (UWorld* World = GetWorld())
	{
		EnableCharacterHitboxesCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Hitbox); 
		
		if (ConfirmHitResult.bBlockingHit)
		{
			if (ConfirmHitResult.Component.IsValid())
			{
				if (UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component))
				{
					for (const auto& Pair : HitCharacter->HitBoxes)
					{
						if (Pair.Value == Box)
						{
							ResetHitBoxes(HitCharacter, CurrentFrame);
							EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

							return FServerSideRewindResult{ true, Pair.Key };
						}
					}
				}
			}
		}
	}
	
	// If no hit, reset and send false hit confirmed result
	ResetHitBoxes(HitCharacter, CurrentFrame);	
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, FName() };
}

FServerSideRewindResult UOFLagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, AOverfrontCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (HitCharacter == nullptr || Package.HitBoxInfos.Num() == 0) return FServerSideRewindResult();
	
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveHitBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_Hitbox;
	PathParams.bTraceWithChannel = true;
	PathParams.ActorsToIgnore.Add(GetOwner());
	
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	
	FPredictProjectilePathResult PathResult;
	
	EnableCharacterHitboxesCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		
	if (PathResult.HitResult.bBlockingHit)
	{
		if (PathResult.HitResult.Component.IsValid())
		{
			if (UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component))
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				
				for (const auto& Pair : HitCharacter->HitBoxes)
				{
					if (Pair.Value == Box)
					{
						ResetHitBoxes(HitCharacter, CurrentFrame);
						EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

						return FServerSideRewindResult{ true, Pair.Key };
					}
				}
			}
		}
	}
	
	ResetHitBoxes(HitCharacter, CurrentFrame);	
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, FName() };
}

FShotgunServerSideRewindResult UOFLagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}
	
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	
	for (const FFramePackage& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		EnableCharacterHitboxesCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
		CurrentFrames.Add(CurrentFrame);
	}

	UWorld* World = GetWorld();
	if (!World) return ShotgunResult;

	for (const FVector_NetQuantize& HitLocation : HitLocations)
	{
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		FHitResult Hit;
		World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Hitbox);

		if (UBoxComponent* HitBox = Cast<UBoxComponent>(Hit.Component))
		{
			if (AOverfrontCharacter* HitCharacter = Cast<AOverfrontCharacter>(Hit.GetActor()))
			{
				for (const auto& Pair : HitCharacter->HitBoxes)
				{
					if (Pair.Value == HitBox)
					{
						ShotgunResult.PelletHits.Add({HitCharacter,Pair.Key});
						break;
					}
				}
			}
		}
	}

	for (const FFramePackage& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;
}

void UOFLagCompensationComponent::CacheBoxPositions(AOverfrontCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	if (HitCharacter->HitBoxes.Num() == 0) return;
	
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (!IsValid(HitBoxPair.Value)) continue;
		FBoxInformation BoxInfo;
		BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
		BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
		BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
		OutFramePackage.HitBoxInfos.Add(HitBoxPair.Key, BoxInfo);
	}
}

void UOFLagCompensationComponent::MoveHitBoxes(AOverfrontCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			if (const FBoxInformation* BoxInfo = Package.HitBoxInfos.Find(HitBoxPair.Key))
			{
				HitBoxPair.Value->SetWorldLocation(BoxInfo->Location);
				HitBoxPair.Value->SetWorldRotation(BoxInfo->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxInfo->BoxExtent);
			}
		}
	}
}
 
void UOFLagCompensationComponent::ResetHitBoxes(AOverfrontCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfos[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfos[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfos[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void UOFLagCompensationComponent::EnableCharacterHitboxesCollision(AOverfrontCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (!HitCharacter) return;

	for (const auto& Pair : HitCharacter->HitBoxes)
	{
		if (Pair.Value)
		{
			Pair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Pair.Value->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
		}
	}
}

void UOFLagCompensationComponent::EnableCharacterMeshCollision(AOverfrontCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void UOFLagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	} else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
}

void UOFLagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<AOverfrontCharacter>(GetOwner()) : Character;
	if (Character == nullptr) return;
	
	Package.Time = GetWorld()->GetTimeSeconds();
	Package.Character = Character;
	
	for (auto& BoxPair : Character->HitBoxes)
	{
		FBoxInformation BoxInfo;
		BoxInfo.Location = BoxPair.Value->GetComponentLocation();
		BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
		BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
		
		Package.HitBoxInfos.Add(BoxPair.Key, BoxInfo);
	}
}
