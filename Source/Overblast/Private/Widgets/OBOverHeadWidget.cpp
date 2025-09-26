// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OBOverHeadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOBOverHeadWidget::SetDisplayText(FString Text)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(Text));
	}
}

void UOBOverHeadWidget::ShowPlayerName(APawn* InPawn)
{
	if (InPawn && InPawn->GetPlayerState())
	{
		FString LocalRoleString = InPawn->GetPlayerState()->GetName();
		SetDisplayText(LocalRoleString);
	}
}

void UOBOverHeadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
