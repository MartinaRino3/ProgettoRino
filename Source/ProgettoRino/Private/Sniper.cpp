// Fill out your copyright notice in the Description page of Project Settings.

#include "Sniper.h"

// Sets default values
ASniper::ASniper()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	}

}

// Called when the game starts or when spawned
void ASniper::BeginPlay()
{
	Super::BeginPlay();

	bIsAlive = true;
	bIsSpawned = false;
	bIsBeingSearch = false;
	MaxDistance = 3;
	MaxAttack = 10;
	MinDamage = 4;
	MaxDamage = 8;
	Life = 20;
	bCanAttack = false;
	
}

void ASniper::SetOwner(int32 NewOwner)  
{
	if (NewOwner)
	{
		Owner = NewOwner;  
	}
}

int32 ASniper::GetOwner()  
{
	return Owner;
}


int32 ASniper::GetRow()
{
	return Row;
}

int32 ASniper::GetCol()
{
	return Col;
}

int32 ASniper::GetLife()
{
	return Life;
}

void ASniper::SetTile(ATile* Tile)
{
	TileSniper = Tile;
}

ATile* ASniper::GetTile()
{
	return TileSniper;
}


// Called every frame
void ASniper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASniper::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASniper::SetPosition(FVector Position)
{
	Row = Position.X;
	Col = Position.Y;
}


