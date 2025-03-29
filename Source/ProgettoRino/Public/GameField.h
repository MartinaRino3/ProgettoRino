// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "TileObstacle.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);

UCLASS()
class PROGETTORINO_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:
	// keeps track of tiles
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	//given a position returns a tile
	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float NextCellPositionMultiplier;

	UPROPERTY(BlueprintAssignable)
	FOnReset OnResetEvent;

	// size of field
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 Size;

	// TSubclassOf template class that provides UClass type safety
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<ATile> TileClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<ATileObstacle> ObstacleClass;

	// tile padding percentage
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CellPadding;

	// tile size
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float TileSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float ObstaclePercentage;

	static const int32 NOT_ASSIGNED = -1;
	
	// Sets default values for this actor's properties
	AGameField();

	// Called when an instance of this class is placed (in editor) or spawned
	virtual void OnConstruction(const FTransform& Transform) override;

	// generate an empty game field
	void GenerateField();
	
	//size of the field
	int32 GetSize ();

	//generate obstacles
	void GenerateObstacles();

	// return a relative position given (x,y) position
	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;

	// return (x,y) position given a relative position
	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	//return how many obstacles are there
	int32 GetNumObstacles(int32 size);

	//check if obstacles in the gamefield are positioned correctly
	bool IsMapFullyConnected();
	
	//probably not more used in .cpp
	void ClearObstacles(ATileObstacle* TmpObstacle);

	//used to place correctly the obstacle to not create island not accessible
	TArray<TSet<FVector2D>> GetConnectedEmptyRegions();

	//probably not more used in .cpp
	TSet<ATile*> GetAdjacentTiles(ATile* Tile);

	ATile* GetTileAt(int32 X, int32 Y);
	
	TArray<ATile*> FindShortestPath(ATile* StartTile, ATile* GoalTile);
	
	TArray<APawn*> GetAttackableEnemies(APawn* Attacker, int32 Player);

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
