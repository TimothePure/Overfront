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
	
	FFRamePackage Package;
	SaveFramePackage(Package);
	// ShowFramePackage(Package, FColor::Orange);
}

void UOFLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFramePackage();
}

void UOFLagCompensationComponent::ShowFramePackage(const FFRamePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfos)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Color, false, 4.f);
	}
}

FServerSideRewindResult UOFLagCompensationComponent::ServerSideRewind(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFRamePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult UOFLagCompensationComponent::ProjectileServerSideRewind(AOverfrontCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFRamePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FFRamePackage UOFLagCompensationComponent::GetFrameToCheck(AOverfrontCharacter* HitCharacter, float HitTime)
{
	if (HitCharacter == nullptr || HitCharacter->GetLagCompensationComponent() == nullptr || 
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr || 
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr) return FFRamePackage();
	
	const TDoubleLinkedList<FFRamePackage>& HitCharacterHistory = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = HitCharacterHistory.GetTail()->GetValue().Time;
	const float NewestHistoryTime = HitCharacterHistory.GetHead()->GetValue().Time;
	
	if (OldestHistoryTime > HitTime) return FFRamePackage(); // Too far back - too laggy for SSR
	if (OldestHistoryTime == HitTime )
	{
		return HitCharacterHistory.GetHead()->GetValue();
	}
	if (NewestHistoryTime <= HitTime) 
	{
		return HitCharacterHistory.GetHead()->GetValue();
	}
	
	TDoubleLinkedList<FFRamePackage>::TDoubleLinkedListNode* Younger = HitCharacterHistory.GetHead();
	TDoubleLinkedList<FFRamePackage>::TDoubleLinkedListNode* Older = Younger;
	
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

void UOFLagCompensationComponent::ServerScoreRequest_Implementation(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, const FName BoneName, float HitTime,  AOFWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	
	if (Character && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{
		if (UOFWeaponDamageType* WeaponDamage = DamageCauser->DamageType->GetDefaultObject<UOFWeaponDamageType>())
		{
			UGameplayStatics::ApplyDamage(HitCharacter, WeaponDamage->DetermineDamageAmount(Confirm.HitBoxName), Character->Controller, DamageCauser, DamageCauser->DamageType);
		}
	}
}

void UOFLagCompensationComponent::ProjectileServerScoreRequest_Implementation(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize100& InitialVelocity, const FName BoneName, float HitTime, TSubclassOf<UDamageType> DamageType)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	
	if (Character && HitCharacter && Confirm.bHitConfirmed)
	{
		if (UOFWeaponDamageType* WeaponDamage = DamageType->GetDefaultObject<UOFWeaponDamageType>())
		{
			UGameplayStatics::ApplyDamage(HitCharacter, WeaponDamage->DetermineDamageAmount(Confirm.HitBoxName), Character->Controller, DamageCauser, DamageCauser->DamageType);
		}
	}
}

FFRamePackage UOFLagCompensationComponent::InterpolateBetweenFrames(const FFRamePackage& OlderFrame, const FFRamePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.0f, 1.0f);
	FFRamePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	
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

FServerSideRewindResult UOFLagCompensationComponent::ConfirmHit(const FFRamePackage& Package, AOverfrontCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr || Package.HitBoxInfos.Num() == 0) return FServerSideRewindResult();
	
	FFRamePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveHitBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	// Enable collision for the head first
	// UBoxComponent* HeadBox = HitCharacter->HitBoxes[FName("head")];
	// HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// HeadBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; 
	if (UWorld* World = GetWorld())
	{
		// World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Hitbox); 
		// if (ConfirmHitResult.bBlockingHit)
		// {
		// 	ResetHitBoxes(HitCharacter, CurrentFrame);	
		// 	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		// 	return FServerSideRewindResult{ true, true };
		// }
		
		for (auto& HitBoxPair : HitCharacter->HitBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
			}
		}
		
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Hitbox); 
		
		if (ConfirmHitResult.bBlockingHit)
		{
			if (ConfirmHitResult.Component.IsValid())
			{
				if (UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component))
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
	}
	// If no hit, reset and send false hit confirmed result
	ResetHitBoxes(HitCharacter, CurrentFrame);	
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, FName() };
}

FServerSideRewindResult UOFLagCompensationComponent::ProjectileConfirmHit(const FFRamePackage& Package, AOverfrontCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (HitCharacter == nullptr || Package.HitBoxInfos.Num() == 0) return FServerSideRewindResult();
	
	FFRamePackage CurrentFrame;
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
	
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
		}
	}
	
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

void UOFLagCompensationComponent::CacheBoxPositions(AOverfrontCharacter* HitCharacter, FFRamePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfos.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void UOFLagCompensationComponent::MoveHitBoxes(AOverfrontCharacter* HitCharacter, const FFRamePackage& Package)
{
	if (HitCharacter == nullptr) return;
	
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfos[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfos[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfos[HitBoxPair.Key].BoxExtent);
		}
	}
}
 
void UOFLagCompensationComponent::ResetHitBoxes(AOverfrontCharacter* HitCharacter, const FFRamePackage& Package)
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
		FFRamePackage ThisFrame;
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
		
		FFRamePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		
		// ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void UOFLagCompensationComponent::SaveFramePackage(FFRamePackage& Package)
{
	Character = Character == nullptr ? Cast<AOverfrontCharacter>(GetOwner()) : Character;
	if (Character == nullptr) return;
	Package.Time = GetWorld()->GetTimeSeconds();
	for (auto& BoxPair : Character->HitBoxes)
	{
		FBoxInformation BoxInfo;
		BoxInfo.Location = BoxPair.Value->GetComponentLocation();
		BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
		BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
		
		Package.HitBoxInfos.Add(BoxPair.Key, BoxInfo);
	}
}
