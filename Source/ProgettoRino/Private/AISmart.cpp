// Fill out your copyright notice in the Description page of Project Settings.

#include "AISmart.h"

#include "IContentBrowserSingleton.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Field/FieldSystemNoiseAlgo.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAISmart::AAISmart()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	
	// default init values
	PlayerNumber = -1;

}

// Called when the game starts or when spawned
void AAISmart::BeginPlay()
{
	Super::BeginPlay();
	BrawlerUnit = nullptr;
	SniperUnit = nullptr;
	NumberActor = 0;
	MoveCounter = 0;
	SpawnFirst = FMath::RandRange(0, 1);
	GameMode = Cast<AMyGameModeBase>(GetWorld()->GetAuthGameMode());
	Field = Cast<AGameField>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameField::StaticClass()));
}

// Called every frame
void AAISmart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAISmart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAISmart::OnTurn()
{
	if (bGameOver)
		return;
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Turn"));
	GameInstance->SetTurnMessage(TEXT("AI (Random) Turn"));

	//Wait some seconds
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
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
			
			GetWorld()->GetTimerManager().SetTimer(Timer, [this](){
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
				
			GetWorld()->GetTimerManager().SetTimer(Timer, [this](){
				if (bTimerFinished)
				{
					bTimerFinished = false;
					PlayBrawler();
					GetWorld()->GetTimerManager().ClearTimer(Timer);
				}
			},1.0f, true);
		}

		GetWorld()->GetTimerManager().SetTimer(WaitEnd, [this](){
			//Wait the movements is end
			if (bTimerFinished)
			{
				GetWorld()->GetTimerManager().ClearTimer(WaitEnd);
				GameMode->TurnNextPlayer();
			}
		},1.0f, true);
		
	},2.0f, false);
	
}

void AAISmart::PlayBrawler()
{
	if (!BrawlerUnit)
	{
		return;
	}
	TArray<ATile*> Path = FindBestPath(BrawlerUnit->GetTile());
	if (Path.Num() > 0)
	{
		GameMode->MoveUnit(BrawlerUnit, Path[Path.Num() - 1]);
		FVector WorldLocation = Field->GetRelativeLocationByXYPosition(Path[Path.Num() - 1]->GetRow(), Path[Path.Num() - 1]->GetColumn());
		BrawlerUnit->SetPosition(FVector (Field->GetXYPositionByRelativeLocation(WorldLocation),0));
		BrawlerUnit->SetTile(Path[Path.Num() - 1]);
		Path[Path.Num() - 1]->SetTileStatus(PlayerNumber,ETileStatus::BRAWLERRANDOM);
	}
	else
	{
		bTimerFinished = true;
	}
	APawn* Target = FindBestAttackTarget(BrawlerUnit);
	if (Target)
	{
		GameMode->Attack(BrawlerUnit, Target);
	}
	AttackableEnemies.Empty();
}

void AAISmart::PlaySniper()
{
	if (!SniperUnit)
		return;
	TArray<ATile*> Path = FindBestPath(SniperUnit->GetTile());
	Path = CheckNeighbours(SniperUnit->GetTile(),Path);
	if (Path.Num() > 0)
	{
		GameMode->MoveUnit(SniperUnit, Path[Path.Num() - 1]);
		FVector WorldLocation = Field->GetRelativeLocationByXYPosition(Path[Path.Num() - 1]->GetRow(), Path[Path.Num() - 1]->GetColumn());
		SniperUnit->SetPosition(FVector (Field->GetXYPositionByRelativeLocation(WorldLocation),0));
		SniperUnit->SetTile(Path[Path.Num() - 1]);
		Path[Path.Num() - 1]->SetTileStatus(PlayerNumber,ETileStatus::SNIPERRANDOM);
	}
	else
	{
		bTimerFinished = true;
	}
	APawn* Target = FindBestAttackTarget(SniperUnit);
	if (Target)
	{
		GameMode->Attack(SniperUnit, Target);
	}
	AttackableEnemies.Empty();
}

TArray<ATile*> AAISmart::CheckNeighbours(ATile* CurrTile,TArray<ATile*> Path)
{
	if (!CurrTile)
		return {};
	TSet<ATile*> AdjacentTiles = Field->GetAdjacentTiles(CurrTile);
	
	for (ATile* Tile : AdjacentTiles)
	{
		if (Tile->GetTileStatus() == ETileStatus::BRAWLERHUMAN || Tile->GetTileStatus() == ETileStatus::SNIPERHUMAN)
		{
			Path.Pop();
		}
	}

	return Path;
}

TArray<ATile*> AAISmart::FindBestPath(ATile* StartTile)
{
	ATile* NearestEnemyTile = GameMode->FindClosestEnemyTile(StartTile, PlayerNumber);
	
	if (!NearestEnemyTile)
		return {};

	//Looking for the nearest empty
	TSet<ATile*> AdjacentTiles = Field->GetAdjacentTiles(NearestEnemyTile);
    TArray<ATile*> ValidTargets;
	for (ATile* Tile : AdjacentTiles)
	{
		if (Tile->GetTileStatus() != ETileStatus::OBSTACLE && Tile->GetOwner() == -1)
		{
			ValidTargets.Add(Tile);
		}
	}

	if (ValidTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna cella libera accanto al nemico!"));
		return {};
	}

	TArray<ATile*> ShortestPath;
	for (ATile* Tile : ValidTargets)
	{
		TArray<ATile*> Path = Field->FindShortestPath(StartTile, Tile);
		if (Path.Num() > 0 && (ShortestPath.Num() == 0 || Path.Num() < ShortestPath.Num()))
		{
			ShortestPath = Path;
		}
	}

	if (ShortestPath.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessun percorso possibile per raggiungere il bersaglio"));
	}

	ShortestPath = GameMode->FindShortestPathReachable(ShortestPath);
	return ShortestPath;
}


APawn* AAISmart::FindBestAttackTarget(APawn* Unit)
{
	APawn* Target = nullptr;
	AttackableEnemies = Field->GetAttackableEnemies(Unit, PlayerNumber);
	
	if (AttackableEnemies.Num() > 0)
	{
		int32 DyingLife = 50;
		for (APawn* PossibleTarget : AttackableEnemies)
		{
			ABrawler* Brawler = Cast<ABrawler>(PossibleTarget);
			ASniper* Sniper = Cast<ASniper>(PossibleTarget);
			if (Brawler)
			{
				if (Brawler->Life < DyingLife)
				{
					DyingLife = Brawler->Life;
					Target = Brawler;
				}
			}
			else if (Sniper)
			{
				if (Sniper->Life < DyingLife)
				{
					DyingLife = Sniper->Life;
					Target = Sniper;
				}
			}
		}
	}
	return Target;
}



int32 AAISmart::GetLifeBrawlerRandom()
{
	return BrawlerUnit->GetLife();
}

int32 AAISmart::GetLifeSniperRandom()
{
	return SniperUnit->GetLife();
}

void AAISmart::ResetAI()
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

void AAISmart::OnWin()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Wins!"));
	GameInstance->SetTurnMessage(TEXT("AI Wins!"));
	FPlatformProcess::Sleep(2.0f);
	
}
