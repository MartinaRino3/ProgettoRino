// Fill out your copyright notice in the Description page of Project Settings.

#include "Brawler.h"

#include "Tile.h"


// Sets default values
ABrawler::ABrawler()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	}

}

// Called when the game starts or when spawned
void ABrawler::BeginPlay()
{
	Super::BeginPlay();

	bIsAlive = true;
	bIsSpawned = false;
	bIsBeingSearch = false;
	MaxDistance = 6;
	MaxAttack = 1;
	MinDamage = 1;
	MaxDamage = 6;
	Life = 40;
	bCanAttack = false;
	
}

void ABrawler::SetOwner(int32 NewOwner)  
{
	if (NewOwner)
	{
		Owner = NewOwner;  
	}
}


int32 ABrawler::GetOwner()  
{
	return Owner;
}

int32 ABrawler::GetRow()
{
	return Row;
}

int32 ABrawler::GetCol()
{
	return Col;
}


int32 ABrawler::GetLife()
{
	return Life;
}

void ABrawler::SetTile(ATile* Tile)
{
	TileBrawler = Tile;
}

ATile* ABrawler::GetTile()
{
	return TileBrawler;
}

// Called every frame
void ABrawler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABrawler::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABrawler::SetPosition(FVector Position)
{
	Row = Position.X;
	Col = Position.Y;
}

