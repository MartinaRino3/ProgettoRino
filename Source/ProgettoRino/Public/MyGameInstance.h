// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

class IPlayerInterface;
/**
 * 
 */
UCLASS()
class PROGETTORINO_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere)
	int32 BrawlerHumanLife = 40;

	UPROPERTY(EditAnywhere)
	int32 SniperHumanLife = 20;

	UPROPERTY(EditAnywhere)
	int32 BrawlerRandomLife = 40;

	UPROPERTY(EditAnywhere)
	int32 SniperRandomLife = 20;

	// message to show every turn
	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	// get the current turn message
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage();

	// set the turn message
	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);

	UFUNCTION(BlueprintCallable)
	void UpdateLifes(APawn* Unit, AActor* Owner);

	UFUNCTION(BlueprintCallable)
	int32 GetBrawlerHumanLife();

	UFUNCTION(BlueprintCallable)
	int32 GetSniperHumanLife();

	UFUNCTION(BlueprintCallable)
	int32 GetBrawlerRandomLife();

	UFUNCTION(BlueprintCallable)
	int32 GetSniperRandomLife();

};
