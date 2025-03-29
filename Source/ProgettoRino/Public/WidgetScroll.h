// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "WidgetScroll.generated.h"

/**
 * 
 */
UCLASS()
class PROGETTORINO_API UWidgetScroll : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(meta = (BindWidget))
	UScrollBox* MoveHistory;

	
	UFUNCTION()
	void AddMoveToHistory(const FString& MoveText);


protected:
	virtual bool Initialize() override;
	
};
