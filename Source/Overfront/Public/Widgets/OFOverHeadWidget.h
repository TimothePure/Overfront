// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OFOverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class OVERFRONT_API UOFOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDisplayText(FString Text);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* InPawn);
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

protected:
	virtual void NativeDestruct() override;
};
