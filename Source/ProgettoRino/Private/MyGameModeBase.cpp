// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameModeBase.h"
#include "AISmart.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "MyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "MyHUDWidget.h"
#include "Kismet/GameplayStatics.h"


AMyGameModeBase::AMyGameModeBase()
{
	PlayerControllerClass = AMyPlayerController::StaticClass();
	DefaultPawnClass = AHumanPlayer::StaticClass();
	FieldSize = 25;
}


void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	//IsGameOver = false;
	
	if (GameFieldClass != nullptr)
	{
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		GField->Size = FieldSize;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game Field is null"));
	}

	HumanPlayer = GetWorld()->GetFirstPlayerController()->GetPawn<AHumanPlayer>();
	//RandomPlayer = GetWorld()->SpawnActor<ARandomPlayer>(FVector(), FRotator());
	RandomPlayer = GetWorld()->SpawnActor<AAISmart>(FVector(), FRotator());

	if (!HumanPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("HumanPlayer è NULL! Controlla se il Pawn esiste"));
	}

	if (!RandomPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("AI è NULL! SpawnActor ha fallito"));
	}
	
	//Inizialized 2 players, 0 = human, 1 = ai
	Players.Add(HumanPlayer);
	Players.Add(RandomPlayer);

	// MiniMax Player
	//auto* AI = GetWorld()->SpawnActor<ATTT_MinimaxPlayer>(FVector(), FRotator());

	//Set the camera
	float CameraPosX = ((GField->TileSize * FieldSize) + ((FieldSize - 1) * GField->TileSize * GField->CellPadding)) * 0.5f;
	float Zposition = 3450.0f;
	FVector CameraPos(CameraPosX, CameraPosX, Zposition);
	HumanPlayer->SetActorLocationAndRotation(CameraPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());

	GameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	
	//Start button
	if (!PlayWidget && MyPlayWidgetClass)
	{
		PlayWidget = CreateWidget<UMyHUDWidget>(GetWorld(), MyPlayWidgetClass);
	}

	//Create scroll box
	if (!MoveBox)
	{
		MoveBox = CreateWidget<UWidgetScroll>(GetWorld(), Box);
		MoveBox->AddToViewport();
		
	}
	
}

//Choose randmly who start and start the game
void AMyGameModeBase::ChoosePlayerAndStartGame()
{
	CurrentPlayer = FMath::RandRange(0, Players.Num() - 1);

	if (!Players.IsValidIndex(CurrentPlayer) || !Players[CurrentPlayer])
	{
		return;
	}

	for (int32 IndexI = 0; IndexI < Players.Num(); IndexI++)
	{
		Players[IndexI]->PlayerNumber = IndexI;
	}
	
	Players[CurrentPlayer]->OnTurn();
}

void AMyGameModeBase::IsSpawnBrawler()
{
	HumanPlayer->bSpawnBrawler = true;
}

void AMyGameModeBase::IsSpawnSniper()
{
	HumanPlayer->bSpawnSniper = true;
}

int32 AMyGameModeBase::GetNextPlayer(int32 Player)
{
	Player++;
	if (!Players.IsValidIndex(Player))
	{
		Player = 0;
	}
	return Player;
}

void AMyGameModeBase::TurnNextPlayer()
{
	CheckColoredTile();
	
	CurrentPlayer = GetNextPlayer(CurrentPlayer);

	Players[CurrentPlayer]->OnTurn();
}

//Spawn brawler and sniper 
void AMyGameModeBase::SpawnActors(int32 Player, FVector &CurrPosition,FString Type)
{
	if (!Players.IsValidIndex(Player) || Players[Player] == nullptr)
	{
		return;
	}
	if (!Players.IsValidIndex(Player) || Player!=CurrentPlayer)
	{
		return;
	}

	//Spawn brawler
	if (Type == "Brawler")
	{
		ABrawler* NewBrawler =  nullptr;
		if (Players[Player]->GetPlayerName() == "HumanPlayer")
		{
			NewBrawler = GetWorld()->SpawnActor<ABrawler>(BrawlerClassHuman, CurrPosition + FVector(0, 0, 10), FRotator::ZeroRotator);
		}
		else if (Players[Player]->GetPlayerName() == "RandomPlayer")
		{
			NewBrawler = GetWorld()->SpawnActor<ABrawler>(BrawlerClassRandom, CurrPosition + FVector(0, 0, 10), FRotator::ZeroRotator);
		}
		
		if (NewBrawler)
		{
			//Set the owner of the brawler
			if (Players[Player] != nullptr)
			{
				NewBrawler->SetOwner(Player);
				NewBrawler->SetPosition(FVector (GField->GetXYPositionByRelativeLocation(CurrPosition),0));
				Players[Player]->SetBrawler(NewBrawler);
				NewBrawler->bIsSpawned = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("C'è qualche errore perchè non esiste il player che sta giocando"));
			}
		}
	}

	//Spawn sniper
	else if (Type == "Sniper")
	{
		ASniper* NewSniper =  nullptr;
		if (Players[Player]->GetPlayerName() == "HumanPlayer")
		{
			NewSniper = GetWorld()->SpawnActor<ASniper>(SniperClassHuman, CurrPosition + FVector(0, 0, 10), FRotator::ZeroRotator);
		}
		else if (Players[Player]->GetPlayerName() == "RandomPlayer")
		{
			NewSniper = GetWorld()->SpawnActor<ASniper>(SniperClassRandom, CurrPosition + FVector(0, 0, 10), FRotator::ZeroRotator);
		}

		if (NewSniper)
		{
			//Set the owner of the brawler
			if (Players[Player] != nullptr)
			{
				NewSniper->SetOwner(Player);
				NewSniper->SetPosition(FVector (GField->GetXYPositionByRelativeLocation(CurrPosition),0));
				Players[Player]->SetSniper(NewSniper);
				NewSniper->bIsSpawned = true;
			}
		}
	}
}

//Turn off the gamefield
void AMyGameModeBase::CheckColoredTile()
{
	for (ATile* Tile : GField->TileArray) 
	{
		if (Tile && Tile->bIsLight()==true)
		{
			Tile->ChangeColor(-1);
			Tile->bIsLightOn = false;
			
		}
	}
}

//Check where a pawn can move
void AMyGameModeBase::CheckActorsMovements(int32 Player, ATile* CurrTile, FString Type)
{
	//Color the tile that I touched
	if (CurrTile->bIsLightOn == false)
	{
		CurrTile->bIsLightOn = true;
		CurrTile -> ChangeColor(1);
	}
	
	//So I cant have two paths
	if (Type == "Brawler" && bIsChangeColored == -1)
	{
		CheckColoredTile();
	}

	if (Type == "Sniper" && bIsChangeColored == 1)
	{
		CheckColoredTile();
	}
	
	int32 MaxDistance = 0;
	if (Type == "Brawler")
	{
		bIsChangeColored = 1;
		MaxDistance = 6;
	}
	else if (Type == "Sniper")
	{
		bIsChangeColored=-1;
		MaxDistance = 3;
	}
	
    // List of ValidTiles where Brawler can be moved
    TSet<ATile*> ValidTiles;

    //Queue for BFS
    TQueue<TPair<ATile*, int32>> Queue;
    Queue.Enqueue(TPair<ATile*, int32>(CurrTile, 0));

    //List of Tile already visited
    TSet<ATile*> Visited;
    Visited.Add(CurrTile);

    while (!Queue.IsEmpty())
    {
        TPair<ATile*, int32> Pair;
        Queue.Dequeue(Pair);

        ATile* Tile = Pair.Key;
        int32 Distance = Pair.Value;

        //Brawler can be moved max in 6 cells, Sniper max 3 cells
        if (Distance >= MaxDistance)
        {
            continue;
        }

        //Get the AdjacentTiles by checking every side
        TSet<ATile*> AdjacentTiles = GField->GetAdjacentTiles(Tile);

    	if (AdjacentTiles.Num() == 0)
    	{
    		UE_LOG(LogTemp, Warning, TEXT("CheckActorsMovements -> Nessuna tile adiacente trovata!"));
    	}

        for (ATile* AdjTile : AdjacentTiles)
        {
            if (Visited.Contains(AdjTile))
                continue;
            //Add the tile that is valid and not yet visited
        	if (AdjTile->GetTileStatus() != ETileStatus::OBSTACLE && AdjTile->GetOwner() == -1)
        	{
        		ValidTiles.Add(AdjTile);
        		Visited.Add(AdjTile);
        		
        		AdjTile->ChangeColor(1);
        		AdjTile->bIsLightOn = true;

        		//Add to the queue with the right distance
        		Queue.Enqueue(TPair<ATile*, int32>(AdjTile, Distance + 1));
        	}
        }
    }
	
}

//Move unit
void AMyGameModeBase::MoveUnit(APawn* Unit, ATile* NewTile)
{
	if (Unit->GetOwner() == HumanPlayer && CurrentPlayer != 0)
	{
		return;
	}

	if (Unit->GetOwner() == RandomPlayer && CurrentPlayer != 1)
	{
		return;
	}
	
	if (!Unit || !NewTile)
	{
		return;
	}

	//Get the old tile
	FVector2D Position = GField->GetXYPositionByRelativeLocation(Unit->GetActorLocation());
	ATile* OldTile = GField->GetTileAt(Position.X, Position.Y);

	//Empty the tile
	if (OldTile)
	{
		OldTile->SetTileStatus(-1, ETileStatus::EMPTY);
	}

	//Stamp in the scroll box
	CreateHistory(OldTile,NewTile,Unit,false,0);

	//Search the best path
	TArray<ATile*> Path = GField->FindShortestPath(OldTile, NewTile);
	
	if (Path.Num() == 0)
	{
		return;
	}
	
	//Check which one 
	ABrawler* Brawler = Cast<ABrawler>(Unit);
	ASniper* Sniper = Cast<ASniper>(Unit);
	
	if (Brawler && (Path.Num()-1 > Brawler->MaxDistance))
	{
		return;
	}

	else if (Sniper && (Path.Num()-1 > Sniper->MaxDistance))
	{
		return;
	}

	//Color the tile for the random player
	if ((Brawler && Brawler->GetOwner() == 1 )|| (Sniper && Sniper->GetOwner() == 1))
	{
		for (ATile* Tile : Path)
		{
			Tile->ChangeColor(1);
			Tile->bIsLightOn = true;
		}
	}

	//Real movement
	GetWorld()->GetTimerManager().SetTimer(MovementTimerHandle, [this, Unit, Path]() mutable
	{
		if (Path.Num() > 0)
		{
			ATile* NextTile = Path[0];
			Path.RemoveAt(0);
			FVector NewLocation = GField->GetRelativeLocationByXYPosition(NextTile->GetRow(), NextTile->GetColumn());
			Unit->SetActorLocation(NewLocation + FVector(0, 0, 10));
		}
		else
		{
			HumanPlayer->bTimerFinished = true;
			RandomPlayer->bTimerFinished = true;
			GetWorld()->GetTimerManager().ClearTimer(MovementTimerHandle);
			CheckColoredTile();
		}
	}, 0.2f, true);
}

//Look for a pawn
APawn* AMyGameModeBase::FindPawn(ATile* StartTile)
{
	if (!StartTile || !GField)
		return nullptr;

	TQueue<ATile*> Queue;
	TSet<ATile*> Visited;
	
	Queue.Enqueue(StartTile);
	Visited.Add(StartTile);

	while (!Queue.IsEmpty())
	{
		ATile* CurrentTile;
		Queue.Dequeue(CurrentTile);

		// Check if in this tile there is something
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
		for (AActor* Actor : FoundActors)
		{
			if (APawn* Pawn = Cast<APawn>(Actor))
			{
				ABrawler* Brawler = Cast<ABrawler>(Pawn);
				ASniper* Sniper = Cast<ASniper>(Pawn);

				//Found the Brawler / Sniper I was searching since the tile are colored
				if ((Brawler && Brawler->bIsBeingSearch==true) || (Sniper && Sniper->bIsBeingSearch==true))
				{
					return Pawn;
				}
			}
		}
		
		//Search in the neighbours
		TSet<ATile*> AdjacentTiles = GField->GetAdjacentTiles(CurrentTile);
		for (ATile* AdjTile : AdjacentTiles)
		{
			if (!Visited.Contains(AdjTile))
			{
				Queue.Enqueue(AdjTile);
				Visited.Add(AdjTile);
			}
		}
	}
	
	return nullptr;
}

APawn* AMyGameModeBase::FindEnemies(ATile* Tile)
{
	if (!Tile || !GField)
	{
		return nullptr;
	}

	//Save (x,y) of the tile 
	FVector2D TilePos(Tile->GetRow(), Tile->GetColumn());
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

	//Check all the actors
	for (AActor* Actor : FoundActors)
	{
		ABrawler* Brawler = Cast<ABrawler>(Actor);
		ASniper* Sniper = Cast<ASniper>(Actor);
		if (!Brawler && !Sniper)
		{
			continue;
		}

		//Look for brawler or sniper
		if (Brawler)
		{
			FVector2D PawnPos (Brawler->GetRow(),Brawler->GetCol());
			if (PawnPos.X == TilePos.X && PawnPos.Y == TilePos.Y)
			{
				return Brawler;
			}
		}
		else if (Sniper)
		{
			FVector2D PawnPos (Sniper->GetRow(),Sniper->GetCol());
			if (PawnPos.X == TilePos.X && PawnPos.Y == TilePos.Y)
			{
				return Sniper;
			}
		}

	}
	return nullptr;
}

void AMyGameModeBase::Attack(APawn* Attacker, APawn* Target)
{	
	if (!Attacker || !Target)
	{
		return;
	}

	//Check the range 
	int32 MinDamage = 0;
	int32 MaxDamage = 0;

	if (ABrawler* Brawler = Cast<ABrawler>(Attacker))
	{
		MinDamage = Brawler->MinDamage;
		MaxDamage = Brawler->MaxDamage;
	}
	else if (ASniper* Sniper = Cast<ASniper>(Attacker))
	{
		MinDamage = Sniper->MinDamage;
		MaxDamage = Sniper->MaxDamage;
	}

	//Choose a damage
	int32 Damage = FMath::RandRange(MinDamage, MaxDamage);

	//If this is a counter attack use another damage
	if (bCounterAttack == true)
	{
		Damage = FMath::RandRange(1, 3);
	}

	
	if (ABrawler* TargetBrawler = Cast<ABrawler>(Target))
	{
		//Decrease life to the correct pawn
		int32 Player = TargetBrawler->GetOwner();
		ATile* TileBrawler = TargetBrawler->GetTile();

		//Stamp the attack
		CreateHistory(nullptr, TileBrawler,Attacker, true , Damage);
		TargetBrawler->Life -= Damage;

		//Check if the pawn die
		if (TargetBrawler->Life <= 0)
		{
			TargetBrawler->Life = 0;
			TargetBrawler->bIsAlive = false;
			TileBrawler->SetTileStatus(-1,ETileStatus::EMPTY);
			if (Player == 0)
			{
				GameInstance->UpdateLifes(TargetBrawler, HumanPlayer);
			}
			else if (Player == 1)
			{
				GameInstance->UpdateLifes(TargetBrawler, RandomPlayer);
			}
			
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, TargetBrawler,Player](){
				Players[Player]->SetBrawler(nullptr);
				TargetBrawler->Destroy();

				//Since the lost check the defeat
				Lose();
				
			},0.5f, false);
		}
		if (TargetBrawler->GetOwner() == 0)
		{
			GameInstance->UpdateLifes(TargetBrawler, HumanPlayer);
		}
		else if (TargetBrawler->GetOwner() == 1)
		{
			GameInstance->UpdateLifes(TargetBrawler, RandomPlayer);
		}
	}
	else if (ASniper* TargetSniper = Cast<ASniper>(Target))
	{
		int32 Player = TargetSniper->GetOwner();
		ATile* TileSniper = TargetSniper->GetTile();
		CreateHistory(nullptr, TileSniper,Attacker, true , Damage);
		TargetSniper->Life -= Damage;

		if (TargetSniper->Life <= 0)
		{
			TargetSniper->Life = 0;
			TargetSniper->bIsAlive = false;
			TileSniper->SetTileStatus(-1,ETileStatus::EMPTY);
			if (Player == 0)
			{
				GameInstance->UpdateLifes(TargetSniper,HumanPlayer);
			}
			else if (Player == 1)
			{
				GameInstance->UpdateLifes(TargetSniper,RandomPlayer);
			}
			
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, TargetSniper,Player](){
				Players[Player]->SetSniper(nullptr);
				TargetSniper->Destroy();
				Lose();
			},0.5f, false);
		}
		if (TileSniper->GetOwner() == 0)
		{
			GameInstance->UpdateLifes(TargetSniper,HumanPlayer);
		}
		else if (TileSniper->GetOwner() == 1)
		{
			GameInstance->UpdateLifes(TargetSniper,RandomPlayer);
		}
	}

	//So it won't repeat 
	if (bCounterAttack == true)
	{
		bCounterAttack = false;
	}
	else
	{
		//Check Counter Attack
		CounterAttack(Attacker,Target);
	}
	
	//Scan the field and "turn off" the tile
	CheckColoredTile();
}

void AMyGameModeBase::CounterAttack(APawn* Attacker, APawn* Target)
{
	//Check for contro attacco
	ASniper* SniperAttacker = Cast<ASniper>(Attacker);

	ASniper* TargetSniper = Cast<ASniper>(Target);
	ABrawler* TargetBrawler = Cast<ABrawler>(Target);
	
	if (!SniperAttacker)
	{
		return;
	}

	//Check if there is the right condition
	if (TargetBrawler)
	{
		ATile* TileAttacker = GField->GetTileAt(SniperAttacker->Row,SniperAttacker->Col);
		ATile* TileTarget = GField->GetTileAt(TargetBrawler->Row,TargetBrawler->Col);

		TSet<ATile*> Neighbors = GField->GetAdjacentTiles(TileTarget);
		for (ATile* Next : Neighbors)
		{
			if (TileAttacker->GetRow() == Next->GetRow() && TileAttacker->GetColumn() == Next->GetColumn())
			{
				bCounterAttack = true;
				Attack(Target, Attacker);
			}
		}
	}
	else if (TargetSniper)
	{
		bCounterAttack = true;
		Attack(Target, Attacker);
	}
}


void AMyGameModeBase::Lose()
{
	for (int32 i = 0; i < Players.Num(); i++)
	{
		if (Players[i]->GetBrawler() == nullptr && Players[i]->GetSniper() == nullptr)
		{
			if (PlayWidget && PlayWidget->ButtonEndTurno)
			{
				PlayWidget->RemoveEndTurn();
			}
			
			if (Players[i]->PlayerNumber == 0)
			{
				RandomPlayer->OnWin();
			}
			else if (Players[i]->PlayerNumber == 1)
			{
				HumanPlayer->OnWin();
			}
		}
	}
	
}

FString AMyGameModeBase::ConvertPosition(int32 X, int32 Y)
{
	int32 Column = X + 1;
	TCHAR Row = 'A' + Y;   
	return FString::Printf(TEXT("%c%d"), Row, Column);
}

//false = move, true = attack
void AMyGameModeBase::CreateHistory(ATile* OldTile,ATile* NewTile,APawn* Pawn, bool Type, int32 Dam)
{
	FString Player;
	FString Unit;
	FString StartPos;
	if (OldTile != nullptr)
	{
		StartPos = ConvertPosition(OldTile->GetRow(), OldTile->GetColumn());
	}
	FString EndPos = ConvertPosition(NewTile->GetRow(), NewTile->GetColumn());
	FString TextMove;

	ABrawler* Brawler = Cast<ABrawler>(Pawn);
	ASniper* Sniper = Cast<ASniper>(Pawn);
	int32 NHuman = HumanPlayer->PlayerNumber;
	int32 NRandom = RandomPlayer->PlayerNumber;

	if (Brawler)
	{
		if (Brawler->GetOwner() == NHuman)
		{
			Player = "HP";
		}
		else if (Brawler->GetOwner() == NRandom)
		{
			Player = "RP";
		}
		Unit = "B";
	}
	else if (Sniper)
	{
		if (Sniper->GetOwner() == NHuman)
		{
			Player = "HP";
		}
		else if (Sniper->GetOwner() == NRandom)
		{
			Player = "RP";
		}
		Unit = "S";
	}

	if (Type == false)
	{
		TextMove = FString::Printf( TEXT("Move : %s : %s %s -> %s"),*Player, *Unit, *StartPos, *EndPos);
	}
	else
	{
		TextMove = FString::Printf( TEXT("Attacco : %s : %s %s %d"),*Player, *Unit, *EndPos,Dam);
	}
	MoveBox->AddMoveToHistory(TextMove);
}


//Used only by AISmart
ATile* AMyGameModeBase::FindClosestEnemyTile(ATile* Tile, int32 Player)
{
	TArray<ATile*> Enemies;
	int32 MinDistance = 1000;
	ATile* ClosestEnemyTile = nullptr;

	if (HumanPlayer->GetBrawler() != nullptr)
	{
		Enemies.Add(HumanPlayer->BrawlerUnit->GetTile());
	}
	if (HumanPlayer->GetSniper() != nullptr)
	{
		Enemies.Add(HumanPlayer->SniperUnit->GetTile());
	}

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

		//Get the AdjacentTiles by checking every side
		TSet<ATile*> AdjacentTiles = GField->GetAdjacentTiles(NewTile);

		if (AdjacentTiles.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("CheckActorsMovements -> Nessuna tile adiacente trovata!"));
		}

		//for each tile I check if there is any pawn
		for (ATile* AdjTile : AdjacentTiles)
		{
			if (Visited.Contains(AdjTile))
				continue;
				
			APawn* PossiblePawn = FindEnemies(AdjTile);
				
			if (PossiblePawn && AdjTile->GetOwner() != Player)
			{
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					ClosestEnemyTile = AdjTile;
				}
			}
			Visited.Add(AdjTile);

			//Add to the queue with the right distance
			Queue.Enqueue(TPair<ATile*, int32>(AdjTile, Distance + 1));
		}
	}
	return ClosestEnemyTile;
}

TArray<ATile*> AMyGameModeBase::FindShortestPathReachable(TArray<ATile*> ShortestPath)
{
	if (ShortestPath.IsEmpty())
		return {};
	
	int32 MaxMovement = 0;
	TArray<ATile*> PathReachable;
	ATile* StartTile = ShortestPath[0];
	
	if (RandomPlayer->BrawlerUnit && StartTile->GetRow() == RandomPlayer->BrawlerUnit->Row && StartTile->GetColumn() == RandomPlayer->BrawlerUnit->Col)
	{
		MaxMovement = RandomPlayer->BrawlerUnit->MaxDistance;
	}
	if (RandomPlayer->SniperUnit && StartTile->GetRow() == RandomPlayer->SniperUnit->Row && StartTile->GetColumn() == RandomPlayer->SniperUnit->Col)
	{
		MaxMovement = RandomPlayer->SniperUnit->MaxDistance;
	}

	if (ShortestPath.Num() > (MaxMovement + 1))
	{
		int32 Step = 0;
		while (Step < (MaxMovement+1))
		{
			PathReachable.Add(ShortestPath[Step]);
			Step++;
		}
		return PathReachable;
	}

	return ShortestPath;
	
}