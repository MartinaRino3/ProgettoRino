// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HumanPlayer.h"

AMyPlayerController::AMyPlayerController()
{
	//Mouse free
	bShowMouseCursor = true;	
	bEnableClickEvents = true;
	//DefaultClickTraceChannel = ECollisionChannel::ECC_Visibility;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(PlayerMappingContext, 0);
	}

}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComp->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AMyPlayerController::ClickOnGrid);
	}
}

void AMyPlayerController::ClickOnGrid()
{
	const auto HumanPlayer = Cast<AHumanPlayer>(GetPawn());
	if (IsValid(HumanPlayer))
	{
		HumanPlayer->OnClick();
	}
}