// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameInstance.h"
#include "PlayerInterface.h"
#include "GameFramework/Pawn.h"
#include "Tile.h"
#include "Brawler.h"
#include "Sniper.h"
#include "MyGameModeBase.h"
#include "GameFramework/Pawn.h"
#include "AISmart.generated.h"


UCLASS()
class PROGETTORINO_API AAISmart : public APawn, public IPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAISmart();

	UMyGameInstance* GameInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actors")
	ABrawler* BrawlerUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actors")
	ASniper* SniperUnit;

	//Set after cause it depends on coin toss
	int32 NumberActor;

	//Max 4 per turno 
	int32 MoveCounter;

	bool bRandomMovements;
	bool bTimerFinished = true;
	int32 SpawnFirst;
	
	TArray<APawn*> AttackableEnemies;
	
	
	FTimerHandle WaitEnd;
	FTimerHandle Timer;
	
	AMyGameModeBase* GameMode;

	AGameField* Field;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// keeps track of turn
	bool IsMyTurn = false;
	bool bGameOver = false;
	

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	//virtual void OnLose() override;
	
	virtual FString GetPlayerName() override {return TEXT("RandomPlayer");};
	virtual void SetBrawler(ABrawler* NewBrawler) override { BrawlerUnit = NewBrawler; }
	virtual ABrawler* GetBrawler() const { return BrawlerUnit; }
	virtual void SetSniper(ASniper* NewSniper) override { SniperUnit = NewSniper; }
	virtual ASniper* GetSniper() const { return SniperUnit; }

	void PlayBrawler();
	void PlaySniper();

	UFUNCTION(BlueprintCallable)
	int32 GetLifeBrawlerRandom();

	UFUNCTION(BlueprintCallable)
	int32 GetLifeSniperRandom();

	void ResetAI();

	TArray<ATile*> FindBestPath(ATile* StartTile);
	APawn* FindBestAttackTarget(APawn* Unit);
	TArray<ATile*> CheckNeighbours(ATile* CurrTile,TArray<ATile*> Path);

};
