// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/OFLagCompensationComponent.h"

#include "Character/OverfrontCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/OFWeapon.h"

UOFLagCompensationComponent::UOFLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOFLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	FFRamePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package, FColor::Orange);
}

void UOFLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFramePackage();
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

void UOFLagCompensationComponent::ShowFramePackage(const FFRamePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfos)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Color, false, 4.f);
	}
}

FServerSideRewindResult UOFLagCompensationComponent::ServerSideRewind(class AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	if (HitCharacter == nullptr || HitCharacter->GetLagCompensationComponent() == nullptr || 
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr || 
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr) return FServerSideRewindResult();
	
	FFRamePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	const TDoubleLinkedList<FFRamePackage>& HitCharacterHistory = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = HitCharacterHistory.GetTail()->GetValue().Time;
	const float NewestHistoryTime = HitCharacterHistory.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime ) return FServerSideRewindResult(); // Too far back - too laggy for SSR
	if (OldestHistoryTime == HitTime )
	{
		FrameToCheck = HitCharacterHistory.GetHead()->GetValue();
	}
	else if (NewestHistoryTime <= HitTime) 
	{
		FrameToCheck = HitCharacterHistory.GetHead()->GetValue();
	}
	else
	{
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
			FrameToCheck = Older->GetValue();
			bShouldInterpolate = false;
		}
		
		if (bShouldInterpolate)
		{
			// Interpolate between Older and Younger
			FrameToCheck = InterpolateBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
		}
	}
	
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void UOFLagCompensationComponent::ServerScoreRequest(AOverfrontCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation, float HitTime,  AOFWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	
	if (Character && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, 20.f, Character->Controller, DamageCauser, UDamageType::StaticClass());
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
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	
	FFRamePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveHitBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	// Enable collision for the head first
	UBoxComponent* HeadBox = HitCharacter->HitBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; 
	if (UWorld* World = GetWorld())
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility); 
		if (ConfirmHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);	
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		} else
		{
			for (auto& HitBoxPair : HitCharacter->HitBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility); 
			
			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);	
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}
	
	ResetHitBoxes(HitCharacter, CurrentFrame);	
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
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
	if (Character == nullptr || Character->HasAuthority()) return;
	
	FFRamePackage ThisFrame;
	if (FrameHistory.Num() <= 1)
	{
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
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		
		// ShowFramePackage(ThisFrame, FColor::Red);
	}
}
