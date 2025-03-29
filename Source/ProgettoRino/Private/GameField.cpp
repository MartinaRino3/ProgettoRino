// Fill out your copyright notice in the Description page of Project Settings.

#include "GameField.h"
#include "Sniper.h"
#include "Brawler.h"
#include "MyGameModeBase.h"
#include "BehaviorTree/Blackboard/BlackboardKeyEnums.h"
#include "Camera/CameraActor.h"
#include "Slate/SGameLayerManager.h"


// Sets default values
AGameField::AGameField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// size of the field (25x25)
	Size = 25;
	// tile dimension
	TileSize = 120.f;
	// tile padding percentage 
	CellPadding =  0.2f;
}

void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	//normalized tilepadding
	NextCellPositionMultiplier = (TileSize + TileSize * CellPadding) / TileSize;
}

int32 AGameField::GetNumObstacles(int32 SizeField)
{
	ObstaclePercentage = 0.25f;
	return FMath::RoundToInt((SizeField * SizeField) * ObstaclePercentage);
}

//Generate field
void AGameField::GenerateField()
{
	for (int32 IndexX = 0; IndexX < Size; IndexX++)
	{
		for (int32 IndexY = 0; IndexY < Size; IndexY++)
		{
			FVector Location = GetRelativeLocationByXYPosition(IndexX, IndexY);
			ATile* Obj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
			Obj->SetRow(IndexX);
			Obj->SetColumn(IndexY);
			const float TileScale = TileSize / 100.f;
			const float Zscaling = 0.1f;
			Obj->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
			Obj->SetGridPosition(IndexX, IndexY);
			TileArray.Add(Obj);
			TileMap.Add(FVector2D(IndexX, IndexY), Obj);
		}
	}
	GenerateObstacles();
}

//Check if there is island
TArray<TSet<FVector2D>> AGameField::GetConnectedEmptyRegions()
{
	TArray<TSet<FVector2D>> Regions;
	TSet<FVector2D> Visited;

	for (const auto& Tile : TileMap)
	{
		if (Tile.Value->GetTileStatus() == ETileStatus::EMPTY && !Visited.Contains(Tile.Key))
		{
			TSet<FVector2D> NewRegion;
			TArray<FVector2D> Stack;
			Stack.Add(Tile.Key);

			while (Stack.Num() > 0)
			{
				FVector2D Current = Stack.Pop();

				if (!Visited.Contains(Current))
				{
					Visited.Add(Current);
					NewRegion.Add(Current);

					//Check each side
					TArray<FVector2D> Neighbors = {
						FVector2D(Current.X + 1, Current.Y),
						FVector2D(Current.X - 1, Current.Y),
						FVector2D(Current.X, Current.Y + 1),
						FVector2D(Current.X, Current.Y - 1)
					};

					for (FVector2D Neighbor : Neighbors)
					{
						if (TileMap.Contains(Neighbor) && 
							TileMap[Neighbor]->GetTileStatus() == ETileStatus::EMPTY &&
							!Visited.Contains(Neighbor))
						{
							Stack.Add(Neighbor);
						}
					}
				}
			}

			//Add to the regions
			Regions.Add(NewRegion);
		}
	}

	return Regions;
}


void AGameField::GenerateObstacles()
{
	int32 NumObstacles = GetNumObstacles(Size);
	TArray<FVector2D> ObstaclePositions;

	while (ObstaclePositions.Num() < NumObstacles)
	{
		int32 X = FMath::RandRange(0, Size - 1);
		int32 Y = FMath::RandRange(0, Size - 1);
		FVector2D Position(X, Y);

		if (ObstaclePositions.Contains(Position))
		{
			continue;
		}
		
		//Spawn the obstacle
		TileMap[Position]->SetTileStatus(-1, ETileStatus::OBSTACLE);
		FVector WorldLocation = GetRelativeLocationByXYPosition(Position.X, Position.Y);
		ATileObstacle* NewObstacle = GetWorld()->SpawnActor<ATileObstacle>(ObstacleClass, WorldLocation, FRotator::ZeroRotator);
		NewObstacle->SetActorScale3D(FVector(TileSize / 100.f, TileSize / 100.f, 0.3f));
		NewObstacle->SetTileStatus(-1, ETileStatus::OBSTACLE);

		//Check there is only 1 region
		if (GetConnectedEmptyRegions().Num() > 1)
		{
			//Otherwise remove the ostacle
			NewObstacle->Destroy();
			TileMap[Position]->SetTileStatus(-1, ETileStatus::EMPTY);
		}
		else
		{
			ObstaclePositions.Add(Position);
		}
	}
}


FVector AGameField::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const
{
	return TileSize * NextCellPositionMultiplier * FVector(InX, InY, 0);
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	const double XPos = Location.X / (TileSize * NextCellPositionMultiplier);
	const double YPos = Location.Y / (TileSize * NextCellPositionMultiplier);
	return FVector2D(XPos, YPos);
}
 

int32 AGameField::GetSize() 
{
	return Size;
}

bool AGameField::IsMapFullyConnected()
{
	TArray<FVector2D> Visited;
	TArray<FVector2D> Stack;
	FVector2D StartTile;

	//Look for the first empty tile 
	for (const auto& Tile : TileMap)
	{
		if (Tile.Value->GetTileStatus() == ETileStatus::EMPTY)
		{
			StartTile = Tile.Key;
			break;
		}
	}
	
	if (StartTile == FVector2D::ZeroVector)
	{
		return true;
	}

	//Check the field
	Stack.Add(StartTile);
	while (Stack.Num() > 0)
	{
		FVector2D Current = Stack[0];
		Stack.RemoveAt(0);

		if (Visited.Contains(Current))
		{
			continue;
		}

		Visited.Add(Current);

		//Check each side
		TArray<FVector2D> Neighbors = {
			FVector2D(Current.X + 1, Current.Y),
			FVector2D(Current.X - 1, Current.Y),
			FVector2D(Current.X, Current.Y + 1),
			FVector2D(Current.X, Current.Y - 1)
		};

		for (FVector2D Neighbor : Neighbors)
		{
			if (TileMap.Contains(Neighbor) && 
				TileMap[Neighbor]->GetTileStatus() == ETileStatus::EMPTY &&
				!Visited.Contains(Neighbor))
			{
				Stack.Add(Neighbor);
			}
		}
	}

	//Compare the number of empty tile visited to the real empty tile
	int32 TotalEmptyTiles = 0;
	for (const auto& Tile : TileMap)
	{
		if (Tile.Value->GetTileStatus() == ETileStatus::EMPTY)
		{
			TotalEmptyTiles++;
		}
	}

	return Visited.Num() == TotalEmptyTiles;
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{	
	Super::BeginPlay();
	GenerateField();
}

//Given the (x,y) coordinate return the tile
ATile* AGameField::GetTileAt(int32 X, int32 Y)
{
	float InX = X;
	float InY = Y;

	//check if are in World Location 
	if (InX>25.0 || InY>25.0)
	{
		FVector2D TilePos2D = GetXYPositionByRelativeLocation(FVector(InX,InY,0));
		if (TileMap.Contains(TilePos2D))
		{
			return *TileMap.Find(TilePos2D);
		}
		return nullptr;
	}
	if (TileMap.Contains(FVector2D(InX,InY)))
	{
		return *TileMap.Find(FVector2D(InX,InY));
	}
	return nullptr;
}

TSet<ATile*> AGameField::GetAdjacentTiles(ATile* Tile)
{
	TSet<ATile*> AdjacentTiles;
	if (!Tile)
		return AdjacentTiles;	

	//Get (x,y) position
	int32 TileX = Tile->GetRow();
	int32 TileY = Tile->GetColumn();

	//Check sides
	ATile* UpTile = GetTileAt(TileX, TileY + 1);
	ATile* DownTile = GetTileAt(TileX, TileY - 1);
	ATile* LeftTile = GetTileAt(TileX - 1, TileY);
	ATile* RightTile = GetTileAt(TileX + 1, TileY);

	//Add only real tile
	if (UpTile)
	{
		AdjacentTiles.Add(UpTile);
	}
	if (DownTile)
	{
		AdjacentTiles.Add(DownTile);
	}
	if (LeftTile)
	{
		AdjacentTiles.Add(LeftTile);
	}
	if (RightTile)
	{
		AdjacentTiles.Add(RightTile);
	}
	return AdjacentTiles;
}

//Look for the best path
TArray<ATile*> AGameField::FindShortestPath(ATile* StartTile, ATile* GoalTile)
{
	bool bIsSniper = false ;
	if (StartTile->GetTileStatus() == ETileStatus::SNIPERRANDOM)
		bIsSniper = true;
	
    TArray<ATile*> Path;
    if (!StartTile || !GoalTile)
    {
        return Path;
    }
	
	if (GoalTile->GetTileStatus() == ETileStatus::OBSTACLE || GoalTile->GetOwner() != -1)
	{
		return Path;
	}

	//Map to track where each tile came from
    TMap<ATile*, ATile*> CameFrom;
    TQueue<ATile*> Frontier;
	
    Frontier.Enqueue(StartTile);
    CameFrom.Add(StartTile, nullptr);

    //BFS 
    while (!Frontier.IsEmpty())
    {
        ATile* Current;
        Frontier.Dequeue(Current);

    	//Stop if I arrive at the goal
        if (Current == GoalTile)
        {
            break;
        }

        TSet<ATile*> Neighbors = GetAdjacentTiles(Current);
        for (ATile* Next : Neighbors)
        {
            //Skip obstacles and pawn
            if (Next->GetTileStatus() == ETileStatus::OBSTACLE || Next->GetOwner() != -1)
            {
                continue;
            }
            if (!CameFrom.Contains(Next))
            {
                Frontier.Enqueue(Next);
                CameFrom.Add(Next, Current);
            }
        }
    }

	//If goalTile was never founded return an empty path
    if (!CameFrom.Contains(GoalTile))
    {
        return Path; 
    }
	
    ATile* Current = GoalTile;
    while (Current != nullptr)
    {
        Path.Insert(Current, 0);

        if (!CameFrom.Contains(Current))
        {
            break;
        }
    	//Move to the previous tile
        Current = CameFrom[Current];
    }
    return Path;
}


TArray<APawn*> AGameField::GetAttackableEnemies(APawn* Attacker, int32 Player)
{	
	TArray<APawn*> AttackableEnemies;
	
	if (!Attacker)
	{
		return AttackableEnemies;
	}

	//Check which range I have
	int32 MaxAttackRange = 0;
	FVector2D AttackerPos;
	if (ASniper* Sniper = Cast<ASniper>(Attacker))
	{
		MaxAttackRange = Sniper->MaxAttack;
		AttackerPos = FVector2d(Sniper->GetRow(),Sniper->GetCol());
	}
	else if (ABrawler* Brawler = Cast<ABrawler>(Attacker))
	{
		MaxAttackRange = Brawler->MaxAttack;
		AttackerPos = FVector2d(Brawler->GetRow(),Brawler->GetCol());
	}

	ATile* Tile = GetTileAt(AttackerPos.X, AttackerPos.Y);

	//Queue for BFS
	TQueue<TPair<ATile*, int32>> Queue;
	Queue.Enqueue(TPair<ATile*, int32>(Tile, 0));

	//List of tile already visited
	TSet<ATile*> Visited;
	Visited.Add(Tile);

	while (!Queue.IsEmpty())
	{
		TPair<ATile*, int32> Pair;
		Queue.Dequeue(Pair);

		ATile* NewTile = Pair.Key;
		int32 Distance = Pair.Value;

		//If is not in my range go on
		if (Distance >= MaxAttackRange)
		{
			continue;
		}

		//Get the AdjacentTiles by checking every side
		TSet<ATile*> AdjacentTiles = GetAdjacentTiles(NewTile);

		if (AdjacentTiles.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetAttackableEnemies -> Nessuna tile adiacente trovata!"));
		}

		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());

		//for each tile I check if there is any pawn
		for (ATile* AdjTile : AdjacentTiles)
		{
			if (Visited.Contains(AdjTile))
				continue;
				
			APawn* PossiblePawn = GameMode->FindEnemies(AdjTile);
				
			if (PossiblePawn && AdjTile->GetOwner() != Player)
			{
				AttackableEnemies.Add(PossiblePawn);
			}
			Visited.Add(AdjTile);

			//Add to the queue with the right distance
			Queue.Enqueue(TPair<ATile*, int32>(AdjTile, Distance + 1));
		}
	}	
	return AttackableEnemies;
}