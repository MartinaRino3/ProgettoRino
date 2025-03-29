// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM()
enum class ETileStatus : uint8
{
	EMPTY     UMETA(DisplayName = "Empty"),
	//OCCUPIED      UMETA(DisplayName = "Occupied"),
	OBSTACLE     UMETA(DisplayName = "Obstacle"),
	BRAWLERHUMAN UMETA(DisplayName = "BrawlerHuman"),
	SNIPERHUMAN UMETA(DisplayName = "SNIPERHUMAN"),
	BRAWLERRANDOM UMETA(DisplayName = "BrawlerRandom"),
	SNIPERRANDOM UMETA(DisplayName = "SniperRandom"),
};

UCLASS()
class PROGETTORINO_API ATile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile();
	ATile(int32 R, int32 C);

	// set the player owner and the status of a tile
	void SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus);

	// get the tile status
	ETileStatus GetTileStatus();

	//get number type based on status 
	int32 GetTileStatusN(ETileStatus TileStatus);

	// get the tile owner
	int32 GetOwner();

	// set the (x, y) position
	void SetGridPosition(const double InX, const double InY);

	// get the (x, y) position
	FVector2D GetGridPosition();

	//get row of the tile
	int32 GetRow();

	//get column of the tile
	int32 GetColumn();
	
	//set row of the tile
	void SetRow(int32 Nrow);

	//set column of the tile
	void SetColumn(int32 Ncolumn);
	
	//should be color some tile -> it doesn't work
	void ChangeColor(int ColorIndex);

	bool bIsLight();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETileStatus Status;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsLightOn = false;
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	// (x, y) position of the tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int Row;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int Column;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	//for the function ToggleTileColor
	UMaterialInstanceDynamic* DynamicMaterial;

};