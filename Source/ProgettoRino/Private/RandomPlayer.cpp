// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomPlayer.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARandomPlayer::ARandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	
	// default init values
	PlayerNumber = -1;
}

// Called when the game starts or when spawned
void ARandomPlayer::BeginPlay()
{
	Super::BeginPlay();
	BrawlerUnit = nullptr;
	SniperUnit = nullptr;
	NumberActor = 0;
	MoveCounter = 0;
	SpawnFirst = FMath::RandRange(0, 1);
}

// Called every frame
void ARandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARandomPlayer::OnTurn()
{
	if (bGameOver)
		return;
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Turn"));
	GameInstance->SetTurnMessage(TEXT("AI (Random) Turn"));

	//Wait some seconds
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
		AGameField* Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
		
		//Check if there are the pawns
		if (NumberActor < 2)
		{
			//Choose a random tile and check if is valid 
			ATile* Tile;
			do
			{
				int32 X = FMath::RandRange(0, Field->Size - 1);
				int32 Y = FMath::RandRange(0, Field->Size - 1);
				Tile = Field->GetTileAt(X,Y);
			}
			while (Tile && Tile->GetTileStatus() != ETileStatus::EMPTY);
			
			FVector WorldLocation = Field->GetRelativeLocationByXYPosition(Tile->GetRow(), Tile->GetColumn());

			//Spawn the brawler and sniper
			if (SpawnFirst == 0)
			{
				if (BrawlerUnit == nullptr)
				{
					GameMode->SpawnActors(PlayerNumber,WorldLocation,"Brawler");
					BrawlerUnit->SetTile(Tile);
					BrawlerUnit->bIsSpawned = true;
					Tile->SetTileStatus(PlayerNumber,ETileStatus::BRAWLERRANDOM);
					NumberActor++;
				}
				else if(SniperUnit == nullptr )
				{
					GameMode->SpawnActors(PlayerNumber,WorldLocation,"Sniper");
					SniperUnit->SetTile(Tile);
					SniperUnit->bIsSpawned = true;
					Tile->SetTileStatus(PlayerNumber,ETileStatus::SNIPERRANDOM);
					NumberActor++;
				}
			}
			else if (SpawnFirst == 1)
			{
				if(SniperUnit == nullptr )
				{
					GameMode->SpawnActors(PlayerNumber,WorldLocation,"Sniper");
					SniperUnit->SetTile(Tile);
					Tile->SetTileStatus(PlayerNumber,ETileStatus::SNIPERRANDOM);
					NumberActor++;
				}
				else if (BrawlerUnit == nullptr)
				{
					GameMode->SpawnActors(PlayerNumber,WorldLocation,"Brawler");
					BrawlerUnit->SetTile(Tile);
					Tile->SetTileStatus(PlayerNumber,ETileStatus::BRAWLERRANDOM);
					NumberActor++;
				}
			}
			GameMode->TurnNextPlayer();
			return;
		}

		//Start to move / attack

		//If there is only one left do immediately the one who is left 
		if (SniperUnit == nullptr)
		{
			PlayBrawler();
			GameMode->TurnNextPlayer();
			return;
		}

		if (BrawlerUnit == nullptr)
		{
			PlaySniper();
			GameMode->TurnNextPlayer();
			return;
		}
			
		//Rand choice to choose which move
		int32 MoveFirst = FMath::RandRange(0, 1);
		if (MoveFirst == 0)
		{
			bTimerFinished = false;
			PlayBrawler();
			
			GetWorld()->GetTimerManager().SetTimer(Timer, [this, GameMode](){
				if (bTimerFinished)
				{
					bTimerFinished = false;
					PlaySniper();
					GetWorld()->GetTimerManager().ClearTimer(Timer);
				}
			},1.0f, true);
		}
		else
		{
			bTimerFinished = false;
			PlaySniper();
				
			GetWorld()->GetTimerManager().SetTimer(Timer, [this, GameMode](){
				if (bTimerFinished)
				{
					bTimerFinished = false;
					PlayBrawler();
					GetWorld()->GetTimerManager().ClearTimer(Timer);
				}
			},1.0f, true);
		}

		GetWorld()->GetTimerManager().SetTimer(WaitEnd, [this, GameMode](){
			//Wait the movements is end
			if (bTimerFinished)
			{
				GetWorld()->GetTimerManager().ClearTimer(WaitEnd);
				GameMode->TurnNextPlayer();
			}
		},1.0f, true);
		
	},2.0f, false);
	
}

void ARandomPlayer::PlayBrawler()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	AGameField* Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));

	//Check if it is still alive or not
	if (BrawlerUnit == nullptr)
		return;

	GameMode->CheckActorsMovements(PlayerNumber,BrawlerUnit->TileBrawler, "Brawler");

	//Save the tiles where the brawler can move
	TArray<ATile*> ValidTiles;
	for (ATile* Tile : Field->TileArray)
	{
		if (Tile->bIsLight())
		{
			ValidTiles.Add(Tile);
		}
	}

	//If there's some
	if (ValidTiles.Num() > 0)
	{
		//Choose one randomly
		ATile* ChosenTile = ValidTiles[FMath::RandRange(0, ValidTiles.Num() - 1)];
		FVector WorldLocation = Field->GetRelativeLocationByXYPosition(ChosenTile->GetRow(), ChosenTile->GetColumn());

		//Move and update
		GameMode->MoveUnit(BrawlerUnit, ChosenTile);
		ChosenTile->SetTileStatus(PlayerNumber,ETileStatus::BRAWLERRANDOM);
		BrawlerUnit->SetPosition(FVector (Field->GetXYPositionByRelativeLocation(WorldLocation),0));
		BrawlerUnit->SetTile(ChosenTile);
	}
	
	//Check if brawler can attack
	AttackableEnemies = Field->GetAttackableEnemies(BrawlerUnit,PlayerNumber);
	if (AttackableEnemies.Num() > 0)
	{
		//Choose which one to attack
		int32 Target = FMath::RandRange(0, AttackableEnemies.Num() - 1);
		GameMode->Attack(BrawlerUnit,AttackableEnemies[Target]);
	}
	
	//Clear
	AttackableEnemies.Empty();
}

void ARandomPlayer::PlaySniper()
{
	AMyGameModeBase* GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	AGameField* Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));

	//Check if it is still alive or not
	if (SniperUnit == nullptr)
		return;
	
	GameMode->CheckActorsMovements(PlayerNumber,SniperUnit->TileSniper, "Sniper");

	//Save the tiles where the brawler can move
	TArray<ATile*> ValidTiles;
	for (ATile* Tile : Field->TileArray)
	{
		if (Tile->bIsLight())
		{
			ValidTiles.Add(Tile);
		}
	}

	//If there's some
	if (ValidTiles.Num() > 0)
	{
		//Choose one randomly
		ATile* ChosenTile = ValidTiles[FMath::RandRange(0, ValidTiles.Num() - 1)];
		FVector WorldLocation = Field->GetRelativeLocationByXYPosition(ChosenTile->GetRow(), ChosenTile->GetColumn());

		//Move and update		
		GameMode->MoveUnit(SniperUnit, ChosenTile);
		ChosenTile->SetTileStatus(PlayerNumber,ETileStatus::SNIPERRANDOM);
		SniperUnit->SetPosition(FVector (Field->GetXYPositionByRelativeLocation(WorldLocation),0));
		SniperUnit->SetTile(ChosenTile);
	}

	//Check if brawler can attack
	AttackableEnemies = Field->GetAttackableEnemies(SniperUnit,PlayerNumber);
	if (AttackableEnemies.Num() > 0)
	{
		//Choose which one to attack
		int32 Target = FMath::RandRange(0, AttackableEnemies.Num() - 1);
		GameMode->Attack(SniperUnit,AttackableEnemies[Target]);
	}
	
	//Clear 
	AttackableEnemies.Empty();
}

int32 ARandomPlayer::GetLifeBrawlerRandom()
{
	return BrawlerUnit->GetLife();
}

int32 ARandomPlayer::GetLifeSniperRandom()
{
	return SniperUnit->GetLife();
}

void ARandomPlayer::ResetAI()
{
	IsMyTurn = false;
	AttackableEnemies.Empty();
	SpawnFirst = FMath::RandRange(0, 1);
	bTimerFinished = true;
	bRandomMovements = false;
	MoveCounter = 0;
	NumberActor = 0;
	SniperUnit = nullptr;
	BrawlerUnit = nullptr;
}

void ARandomPlayer::OnWin()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Wins!"));
	GameInstance->SetTurnMessage(TEXT("AI Wins!"));
	FPlatformProcess::Sleep(2.0f);
	
}
