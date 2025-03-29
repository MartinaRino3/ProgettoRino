// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystemInterface.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "MyPlayerController.generated.h"


UCLASS()
class PROGETTORINO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* ClickAction;

	void ClickOnGrid();
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
