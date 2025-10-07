// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OFOverHeadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOFOverHeadWidget::SetDisplayText(FString Text)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(Text));
	}
}

void UOFOverHeadWidget::ShowPlayerName(APawn* InPawn)
{
	if (InPawn && InPawn->GetPlayerState())
	{
		FString LocalRoleString = InPawn->GetPlayerState()->GetName();
		SetDisplayText(LocalRoleString);
	}
}

void UOFOverHeadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
