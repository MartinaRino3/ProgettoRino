// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Brawler.h"
#include "MyGameInstance.h"
#include "Sniper.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "HumanPlayer.generated.h"

class ATile;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndTurn);

UCLASS()
class PROGETTORINO_API AHumanPlayer : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHumanPlayer();

	// camera component attacched to player pawn
	UCameraComponent* Camera;

	// game instance reference
	UMyGameInstance* GameInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actors")
	ABrawler* BrawlerUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actors")
	ASniper* SniperUnit;
	
	UPROPERTY(BlueprintAssignable)
	FEndTurn EndTurn;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> EndTurnWidgetClass;
	
	//Set after cause it depends on coin toss
	int32 NumberActor = 0;

	//Max 4 per turno
	int32 MoveCounter;
	
	TArray<APawn*> AttackableEnemiesBrawler;
	TArray<APawn*> AttackableEnemiesSniper;

	FTimerHandle WaitEnd;
	bool bTimerFinished = true;

	//to avoid move the same pawn two times in one turn 
	bool bIsBrawlerMoved;
	bool bIsSniperMoved;
	bool bIsBrawlerAttack;
	bool bIsSniperAttack;
	bool bGameOver = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// keeps track of turn
	bool IsMyTurn = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	//virtual void OnLose() override;
	
	virtual FString GetPlayerName() override {return TEXT("HumanPlayer");};
	virtual void SetBrawler(ABrawler* NewBrawler) override{BrawlerUnit = NewBrawler;}
	ABrawler* GetBrawler() const { return BrawlerUnit; }
	virtual void SetSniper(ASniper* NewSniper) override{SniperUnit = NewSniper;}
	virtual ASniper* GetSniper() const { return SniperUnit; }
	void ResetPawnMoved();
	void CheckEndTurn();

	void SpawnBrawler();
	void SpawnSniper();

	// called on left mouse click (binding)
	UFUNCTION()
	void OnClick();

	UFUNCTION(BlueprintCallable)
	void EndPlayerTurn();
	
	
	UFUNCTION(BlueprintCallable)
	int32 GetLifeBrawlerHuman();

	UFUNCTION(BlueprintCallable)
	int32 GetLifeSniperHuman();
	
	bool bSpawnBrawler = false;
	bool bSpawnSniper = false;

	void ResetHuman();

};
