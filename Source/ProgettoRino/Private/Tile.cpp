// Fill out your copyright notice in the Description page of Project Settings.

#include "Tile.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("M_Tile"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	Status = ETileStatus::EMPTY;
	PlayerOwner = -1;
	Row = 0;
	Column = 0;
	TileGridPosition = FVector2D(Row, Column);

}

//Use when I have to change the status of a tile
void ATile::SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus)
{
	PlayerOwner = TileOwner;
	Status = TileStatus;
}

ETileStatus ATile::GetTileStatus()
{
	return Status;
}

int32 ATile::GetTileStatusN(ETileStatus TileStatus)
{
	switch (TileStatus)
	{
		case ETileStatus::EMPTY:
			return 0;
		case ETileStatus::BRAWLERHUMAN:
			return 1;
		case ETileStatus::SNIPERHUMAN:
			return 2;
		case ETileStatus::OBSTACLE:
			return 3;
		case ETileStatus::BRAWLERRANDOM:
			return 4;
		case ETileStatus::SNIPERRANDOM:
			return 5;
		default:
			return -1;
	}
}

int32 ATile::GetOwner()
{
	return PlayerOwner;
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ATile::GetGridPosition()
{
	return TileGridPosition;
}

int32 ATile::GetRow()
{
	return Row;
}

int32 ATile::GetColumn()
{
	return Column;
}

void ATile::SetRow(int32 Nrow)
{
	Row = Nrow;
}

void ATile::SetColumn(int32 Ncolumn)
{
	Column = Ncolumn;
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
}

//Change the color of the tile 
void ATile::ChangeColor(int ColorIndex)
{
	if (!DynamicMaterial)
	{
		DynamicMaterial = StaticMeshComponent->CreateDynamicMaterialInstance(0);
		if (!DynamicMaterial)
		{
			return;
		}
	}

	//Pairing color and id
	FLinearColor ColorToSet;
	switch (ColorIndex)
	{
	case 0:
		ColorToSet = FLinearColor::Red;
		break;
	case 1:
		ColorToSet = FLinearColor::Green;
		break;
	default:
		ColorToSet = FLinearColor::White;
		break;
	}

	//Set the color
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), ColorToSet);
}

//Check if the tile isn't white
bool ATile::bIsLight()
{
	return bIsLightOn;
}