// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInterface.h"
#include "Tile.h"
#include "GameFramework/Pawn.h"
#include "Sniper.generated.h"


UCLASS()
class PROGETTORINO_API ASniper : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASniper();
	
	bool bIsSpawned;
	bool bIsAlive;
	int32 Row;
	int32 Col;
	int32 Owner;
	bool bCanAttack;
	bool bIsBeingSearch;
	int32 MaxDistance;
	int32 MaxAttack;
	int32 MinDamage;
	int32 MaxDamage;
	int32 Life;
	ATile* TileSniper;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//set the position on the field
	void SetPosition(FVector Position);

	//set the owner of that brawler
	void SetOwner(int32 NewOwner);

	//get the owner
	int32 GetOwner() ;
	
	int32 GetRow();
	int32 GetCol();
	int32 GetLife();

	void SetTile(ATile* Tile);
	ATile* GetTile();
};
