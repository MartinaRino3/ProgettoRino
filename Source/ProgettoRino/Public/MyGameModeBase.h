// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameField.h"
#include "PlayerInterface.h"
#include "MyHUDWidget.h"
#include "WidgetScroll.h"
#include "HAL/ThreadSafeCounter.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

class UMyGameInstance;
class ARandomPlayer;
class AHumanPlayer;
class ASniper;
class ABrawler;
class AAISmart;


UCLASS()
class PROGETTORINO_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// array of player interfaces
	TArray<IPlayerInterface*> Players;

	AHumanPlayer* HumanPlayer;
	//ARandomPlayer* RandomPlayer;
	AAISmart* RandomPlayer;
	
	int32 CurrentPlayer;
	
	// TSubclassOf is a template class that provides UClass type safety.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	UPROPERTY(EditDefaultsOnly, Category = "Brawler")
	TSubclassOf<ABrawler> BrawlerClassHuman;

	UPROPERTY(EditDefaultsOnly, Category = "Brawler")
	TSubclassOf<ABrawler> BrawlerClassRandom;

	UPROPERTY(EditDefaultsOnly, Category = "Sniper")
	TSubclassOf<ASniper> SniperClassHuman;

	UPROPERTY(EditDefaultsOnly, Category = "Sniper")
	TSubclassOf<ASniper> SniperClassRandom;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> MyPlayWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UWidgetScroll> Box;

	//Per la grafica
	UPROPERTY(EditAnywhere)
	int32 BrawlerHuman = 40;

	UPROPERTY(EditAnywhere)
	int32 SniperHuman = 20;

	UPROPERTY(EditAnywhere)
	int32 BrawlerRandom = 40;

	UPROPERTY(EditAnywhere)
	int32 SniperRandom = 20;
	

	// field size
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	// reference to a GameField object
	UPROPERTY(VisibleAnywhere)
	AGameField* GField;

	UPROPERTY()
	UMyHUDWidget* PlayWidget;

	UPROPERTY()
	UWidgetScroll* MoveBox;


	FTimerHandle MovementTimerHandle;

	//1 brawler, -1 sniper
	int32 bIsChangeColored = 0;

	UMyGameInstance* GameInstance;

	bool bIsBrawlerSpawning = false;
	bool bIsSniperSpawning = false;
	bool bTimerFinished = false;
	bool bCounterAttack = false;
	bool bAISmart = false;
	
	AMyGameModeBase();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// called at the start of the game
	void ChoosePlayerAndStartGame();

	// get the next player index
	int32 GetNextPlayer(int32 Player);

	// called at the end of the game turn
	void TurnNextPlayer();

	void SpawnActors(int32 Player, FVector &CurrPosition,FString Type);

	void CheckActorsMovements(int32 Player, ATile* CurrTile, FString Type);

	void MoveUnit(APawn* Unit,ATile* NewTile);

	APawn* FindPawn(ATile* StartTile);

	void Attack(APawn* Attacker, APawn* Target);

	APawn* FindEnemies(ATile* StartTile);

	void CheckColoredTile();

	void IsSpawnBrawler();

	void IsSpawnSniper();

	void Lose();

	FString ConvertPosition(int32 X, int32 Y);

	void CreateHistory(ATile* OldTile,ATile* NewTile,APawn* Pawn, bool Type, int32 Dam);

	void CounterAttack(APawn* Attacker, APawn* Target);

	ATile* FindClosestEnemyTile(ATile* Tile, int32 Player);
	
	TArray<ATile*> FindShortestPathReachable(TArray<ATile*> ShortestPath);
	
};