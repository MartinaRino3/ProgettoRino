// Fill out your copyright notice in the Description page of Project Settings.

#include "HumanPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Widget.h"


//NON LEVARE 
#include "Tile.h"
#include "MyGameModeBase.h"


AHumanPlayer::AHumanPlayer()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// create a camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//set the camera as RootComponent
	SetRootComponent(Camera);
	// get the game instance reference
	GameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	// default init values
	PlayerNumber = -1;

}

// Called when the game starts or when spawned
void AHumanPlayer::BeginPlay()
{
	Super::BeginPlay();
	BrawlerUnit = nullptr;
	SniperUnit = nullptr;
	NumberActor = 0;
	MoveCounter = 0;
}

// Called every frame
void AHumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AHumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//Called when is my turn 
void AHumanPlayer::OnTurn()
{
	IsMyTurn = true;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Your Turn"));
	GameInstance->SetTurnMessage(TEXT("Human Turn"));
}

//When I win 
void AHumanPlayer::OnWin()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("You Win!"));
	GameInstance->SetTurnMessage(TEXT("Human Wins!"));
	bGameOver = true;
	FPlatformProcess::Sleep(2.0f);
}

//When Human click a tile
void AHumanPlayer::OnClick()
{
	//Structure containing information about one hit of a trace, such as point of impact and surface normal at that point
	FHitResult Hit = FHitResult(ForceInit);

	// GetHitResultUnderCursor function sends a ray from the mouse position and gives the corresponding hit results
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	AGameField* Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
	
	if (Hit.bBlockingHit && IsMyTurn)
	{
		//If the game is over or some pawns are moving return 
		if (bGameOver)
		{
			return;
		}
		if (!bTimerFinished)
		{
			return;
		}
		
		//I need to touch a tile 
		if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))	
		{				
			//since CurrTile has (x,y) coordinate, World Location is with real location
			FVector WorldLocation = Field->GetRelativeLocationByXYPosition(CurrTile->GetRow(), CurrTile->GetColumn());
			
			//Use for move the pawn
			if (CurrTile->bIsLight() == true)
			{				
				//Check which generate the light
				if (BrawlerUnit && BrawlerUnit->bIsBeingSearch)
				{
					//Move and update data
					bTimerFinished = false;
					GameMode->MoveUnit(BrawlerUnit,CurrTile);
					CurrTile->SetTileStatus(PlayerNumber,ETileStatus::BRAWLERHUMAN);
					BrawlerUnit->SetPosition(FVector (Field->GetXYPositionByRelativeLocation(WorldLocation),0));
					BrawlerUnit->SetTile(CurrTile);
					BrawlerUnit->bIsBeingSearch = false;
					bIsBrawlerMoved = true;
					MoveCounter++;
				}
				
				else if (SniperUnit && SniperUnit ->bIsBeingSearch)
				{
					bTimerFinished = false;
					GameMode->MoveUnit(SniperUnit,CurrTile);
					CurrTile->SetTileStatus(PlayerNumber,ETileStatus::SNIPERHUMAN);
					SniperUnit->SetPosition(FVector (Field->GetXYPositionByRelativeLocation(WorldLocation),0));
					SniperUnit->SetTile(CurrTile);
					SniperUnit->bIsBeingSearch = false;
					bIsSniperMoved = true;
					MoveCounter++;
				}
			}

			//Use for spawn brawler and sniper
			else if (CurrTile->GetTileStatus() == ETileStatus::EMPTY)
			{
				//Check if are already spawn and which button is being clicked
				if (BrawlerUnit==nullptr && NumberActor < 2 && bSpawnBrawler == true)
				{
					bTimerFinished = false;
					GameMode->SpawnActors(PlayerNumber,WorldLocation,"Brawler");
					BrawlerUnit->SetTile(CurrTile);
					BrawlerUnit->bIsBeingSearch = false;
					CurrTile->SetTileStatus(PlayerNumber,ETileStatus::BRAWLERHUMAN);
					NumberActor++;
					bTimerFinished = true;
					EndPlayerTurn();
				}
				else if(SniperUnit==nullptr && NumberActor < 2 && bSpawnSniper == true)
				{
					bTimerFinished = false;
					GameMode->SpawnActors(PlayerNumber,WorldLocation,"Sniper");
					SniperUnit->SetTile(CurrTile);
					SniperUnit->bIsBeingSearch = false;
					CurrTile->SetTileStatus(PlayerNumber,ETileStatus::SNIPERHUMAN);
					NumberActor++;
					bTimerFinished = true;
					EndPlayerTurn();
				}
			}

			//You can't move in an obstacle
			else if(CurrTile->GetTileStatus() == ETileStatus::OBSTACLE)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Toccare una cella libera! "));
			}

			//Try to attack without choose the pawn
			else if (CurrTile->GetTileStatus() == ETileStatus::SNIPERRANDOM || CurrTile->GetTileStatus() == ETileStatus::BRAWLERRANDOM)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Prima seleziona con chi vuoi attaccare! "));
			}
		}

		//If any brawler has been touched 
		else if (ABrawler* Brawler = Cast<ABrawler>(Hit.GetActor()))
		{
			//You need to put the two actors
			if (NumberActor < 2)
				return;

			//Get the tile where is the brawler
			ATile* TileBrawler = Field->GetTileAt(Brawler->Row,Brawler->Col);
			if (!TileBrawler)
			{
				return;
			}

			//If the brawler touched is mine
			if (BrawlerUnit && Brawler->GetRow() == BrawlerUnit->Row && Brawler->GetCol() == BrawlerUnit->Col && BrawlerUnit->bIsAlive)
			{
				//If they are light then I turn off
				if (BrawlerUnit->GetTile()->bIsLight())
				{
					GameMode->CheckColoredTile();
					BrawlerUnit->bIsBeingSearch = false;
					return;
				}

				//Can't use the same in one turn
				if (bIsBrawlerAttack)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Errore: Non puoi usare lo stesso pawn due volte in un turno"));
					return;
				}				

				//Color the tile I touch
				BrawlerUnit->GetTile()->bIsLightOn = true;
				BrawlerUnit->GetTile()-> ChangeColor(1);
				BrawlerUnit->bIsBeingSearch = true;

				//Can't be search in the same moment 
				if (SniperUnit && SniperUnit -> bIsBeingSearch)
				{
					SniperUnit->bIsBeingSearch = false;
				}
								
				//If it hasn't been moved yet
				if (!bIsBrawlerMoved)
				{
					GameMode->CheckActorsMovements(PlayerNumber,TileBrawler, "Brawler");
				}						

				//Okay cause if it has already attack I know above this 
				AttackableEnemiesBrawler = Field->GetAttackableEnemies(BrawlerUnit,PlayerNumber);
				if (AttackableEnemiesBrawler.Num() > 0)
				{
					//Change the color of the tile where there is an enemy
					for (APawn* Unit : AttackableEnemiesBrawler)
					{
						if (Unit)
						{
							FVector2D UnitPos = Field->GetXYPositionByRelativeLocation(Unit->GetActorLocation());
							ATile* TileUnit = Field->GetTileAt(UnitPos.X,UnitPos.Y);
							TileUnit->ChangeColor(0);
							TileUnit->bIsLightOn = true;
						}
					}

					//Since it has someone to attack 
					BrawlerUnit->bCanAttack = true;
					if (SniperUnit && SniperUnit->bCanAttack)
					{
						SniperUnit->bCanAttack = false;
					}
				}
			}

			//If the brawler isn't mine
			else
			{
				//If the brawler can be attacked by my brawler (cause I touched my brawler before)
				if (BrawlerUnit && BrawlerUnit->bCanAttack == true)
				{
					for (APawn* Target : AttackableEnemiesBrawler)
					{
						//if the brawler i touched is the same as the brawler can attack 
						FVector2D TargetPos = Field->GetXYPositionByRelativeLocation(Target->GetActorLocation());
						if (TargetPos.X == TileBrawler->GetRow() && TargetPos.Y == TileBrawler->GetColumn())
						{
							GameMode->Attack(BrawlerUnit,Target);

							//if a pawn attack can't move anymore
							bIsBrawlerAttack = true;
							bIsBrawlerMoved = true;

							MoveCounter++;
							if (BrawlerUnit)
							{
								BrawlerUnit->bCanAttack = false;
							}
						}
					}
					AttackableEnemiesBrawler.Empty();
				}

				if (SniperUnit && SniperUnit->bCanAttack == true)
				{
					for (APawn* Target : AttackableEnemiesSniper)
					{
						FVector2D TargetPos = Field->GetXYPositionByRelativeLocation(Target->GetActorLocation());
						if (TargetPos.X == TileBrawler->GetRow() && TargetPos.Y == TileBrawler->GetColumn())
						{
							GameMode->Attack(SniperUnit,Target);
							bIsSniperMoved = true;
							bIsSniperAttack = true;

							MoveCounter++;
							if (SniperUnit)
							{
								SniperUnit->bCanAttack = false;
							}
						}
					}
					AttackableEnemiesSniper.Empty();
				}
				else
				{
					
					UE_LOG(LogTemp, Warning, TEXT("Cella non tua"));
				}
			}
		}

		//If any sniper has been touched
		else if (ASniper* Sniper = Cast<ASniper>(Hit.GetActor()))
		{			
			//You need to put the two actors
			if (NumberActor < 2)
				return;
			
			ATile* TileSniper = Field->GetTileAt(Sniper->Row,Sniper->Col);

			//Check if the Sniper touched is mine
			if (SniperUnit && Sniper->GetRow() == SniperUnit->GetRow() && Sniper->GetCol() == SniperUnit->GetCol() && SniperUnit->bIsAlive)
			{
				
				//If they are light then I turn off
				if (SniperUnit->GetTile()->bIsLight())
				{
					GameMode->CheckColoredTile();
					SniperUnit->bIsBeingSearch = false;
					return;
				}

				//Can't use the same in one tune
				if (bIsSniperAttack)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Errore: Non puoi usare lo stesso pawn due volte in un turno"));
					return;
				}

				//Color the tile that I touched
				SniperUnit->GetTile()->bIsLightOn = true;
				SniperUnit->GetTile()-> ChangeColor(1);
				SniperUnit->bIsBeingSearch = true;

				//Can't be search in the same moment
				if (BrawlerUnit && BrawlerUnit->bIsBeingSearch)
				{
					BrawlerUnit->bIsBeingSearch = false;
				}

				//If it hasn't been moved yet
				if (!bIsSniperMoved)
				{
					GameMode->CheckActorsMovements(PlayerNumber,TileSniper,"Sniper");
				}

				//Okay cause if it has already attack I know above this 
				AttackableEnemiesSniper = Field->GetAttackableEnemies(SniperUnit,PlayerNumber);
				if (AttackableEnemiesSniper.Num() > 0)
				{
					//Change the color of the tile where there is an enemy
					for (APawn* Unit : AttackableEnemiesSniper)
					{
						if (Unit)
						{
							FVector2D UnitPos = Field->GetXYPositionByRelativeLocation(Unit->GetActorLocation());
							ATile* TileUnit = Field->GetTileAt(UnitPos.X,UnitPos.Y);
							TileUnit->ChangeColor(0);
							TileUnit->bIsLightOn = true;
						}
					}

					//Since it has someone to attack 
					SniperUnit->bCanAttack = true;
					if (BrawlerUnit && BrawlerUnit->bCanAttack)
					{
						BrawlerUnit->bCanAttack = false;
					}
				}
			}

			//If the sniper isn't mine
			else
			{
				//If the brawler can be attacked by my brawler (cause I touched my brawler before)
				if (BrawlerUnit && BrawlerUnit->bCanAttack == true)
				{
					for (APawn* Target : AttackableEnemiesBrawler)
					{
						//if the brawler i touched is the same as the brawler can attack 
						FVector2D TargetPos = Field->GetXYPositionByRelativeLocation(Target->GetActorLocation());
						if (TargetPos.X == TileSniper->GetRow() && TargetPos.Y == TileSniper->GetColumn())
						{
							GameMode->Attack(BrawlerUnit,Target);

							//if a pawn attack can't move anymore
							bIsBrawlerAttack = true;
							bIsBrawlerMoved = true;
							
							MoveCounter++;
							if (BrawlerUnit)
							{
								BrawlerUnit->bCanAttack = false;
							}
						}
					}
					AttackableEnemiesBrawler.Empty();
				}

				if (SniperUnit && SniperUnit->bCanAttack == true)
				{
					for (APawn* Target : AttackableEnemiesSniper)
					{
						FVector2D TargetPos = Field->GetXYPositionByRelativeLocation(Target->GetActorLocation());
						if (TargetPos.X == TileSniper->GetRow() && TargetPos.Y == TileSniper->GetColumn())
						{
							GameMode->Attack(SniperUnit,Target);
							bIsSniperMoved = true;
							bIsSniperAttack = true;
							
							MoveCounter++;
							if (SniperUnit)
							{
								SniperUnit->bCanAttack = false;
							}
						}
					}
					AttackableEnemiesSniper.Empty();
				}
				
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Cella non tua"));
				}
			}
		}
		//if I use all the moves available
		
		GetWorld()->GetTimerManager().SetTimer(WaitEnd, [this, GameMode](){
			//Wait the movements is end
			if (bTimerFinished)
			{
				GetWorld()->GetTimerManager().ClearTimer(WaitEnd);
				CheckEndTurn();
			}
		},1.0f, true);
	}
}


//Check if the turn is end and in case reset 
void AHumanPlayer::CheckEndTurn()
{	
	if (MoveCounter == 4)
	{
		EndPlayerTurn();
	}
	if (bIsBrawlerAttack == true && bIsSniperAttack == true)
	{
		EndPlayerTurn();
	}
	if (bIsBrawlerMoved && bIsSniperMoved)
	{		
		AGameField* Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
		if (SniperUnit)
		{
			AttackableEnemiesSniper = Field->GetAttackableEnemies(SniperUnit,PlayerNumber);
		}
		if (BrawlerUnit)
		{
			AttackableEnemiesBrawler = Field->GetAttackableEnemies(BrawlerUnit,PlayerNumber);
		}
		
		if (AttackableEnemiesBrawler.Num() == 0 && AttackableEnemiesSniper.Num() == 0)
		{
			EndPlayerTurn();
		}
		else if (bIsBrawlerAttack && AttackableEnemiesSniper.Num() == 0)
		{
			EndPlayerTurn();
		}
		else if (bIsSniperAttack && AttackableEnemiesBrawler.Num() == 0)
		{
			EndPlayerTurn();
		}
	}
	if (bIsBrawlerAttack && SniperUnit == nullptr || bIsSniperAttack && BrawlerUnit == nullptr)
	{
		EndPlayerTurn();
	}
	if (bIsBrawlerMoved && SniperUnit == nullptr)
	{
		AGameField* Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
		TSet<ATile*> AdjacentTiles = Field->GetAdjacentTiles(BrawlerUnit->GetTile());
		bool Founded = false;
		for (ATile* Tile : AdjacentTiles)
		{
			if (Tile->GetTileStatus() == ETileStatus::BRAWLERRANDOM || Tile->GetTileStatus() == ETileStatus::SNIPERRANDOM)
			{
				Founded = true;
			}
		}
		if (Founded)
			return;
		EndPlayerTurn();
	}
	if (bIsSniperMoved && BrawlerUnit == nullptr)
	{
		AGameField* Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
		AttackableEnemiesSniper = Field->GetAttackableEnemies(SniperUnit,PlayerNumber);
		if (AttackableEnemiesSniper.Num() > 0)
			return;
		EndPlayerTurn();
	}
}

void AHumanPlayer::EndPlayerTurn()
{
	MoveCounter = 0;
	AttackableEnemiesSniper.Empty();
	AttackableEnemiesBrawler.Empty();
	
	if (SniperUnit)
	{
		SniperUnit->bIsBeingSearch = false;
		SniperUnit->bCanAttack = false;
	}
	if (BrawlerUnit)
	{
		BrawlerUnit->bIsBeingSearch = false;
		BrawlerUnit->bCanAttack = false;
	}
	
	bIsSniperMoved = false;
	bIsBrawlerMoved = false;
	bIsSniperAttack = false;
	bIsBrawlerAttack = false;
	
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode->CurrentPlayer == PlayerNumber)
	{
		IsMyTurn = false;
		EndTurn.Broadcast();
		GameMode->TurnNextPlayer();
	}
}

int32 AHumanPlayer::GetLifeBrawlerHuman()
{
	return BrawlerUnit->GetLife();
}

int32 AHumanPlayer::GetLifeSniperHuman()
{
	return SniperUnit->GetLife();
}

void AHumanPlayer::ResetHuman()
{
	BrawlerUnit = nullptr;
	SniperUnit = nullptr;
	NumberActor = 0;
	MoveCounter = 0;
	AttackableEnemiesBrawler.Empty();
	AttackableEnemiesSniper.Empty();
	bTimerFinished = true;
	bIsBrawlerMoved = false;
	bIsSniperMoved = false;
	bIsBrawlerAttack = false;
	bIsSniperAttack = false;
	bGameOver = false;
	IsMyTurn = false;
	bSpawnBrawler = false;
	bSpawnSniper = false;
}
